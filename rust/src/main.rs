mod kdtree;
mod embed;

use actix_web::{web, App, HttpServer, Result};
use actix_cors::Cors;
use serde::{Deserialize, Serialize};
use regex::Regex;
use kdtree::KDTreeVectorDB;

#[derive(Deserialize)]
struct UploadRequest {
    resume: String,
}

#[derive(Deserialize)]
struct QueryRequest {
    query: String,
    k: usize,
}

#[derive(Serialize)]
struct QueryResult {
    id: usize,
    distance: f32,
    metadata: serde_json::Value,
}

fn vectorize_resume(resume_text: &str) -> Vec<(Vec<f32>, String)> {
    let re = Regex::new(r"[^\.!\?]+[\.!\?]+").unwrap();
    let chunks: Vec<String> = re.find_iter(resume_text).map(|m| m.as_str().to_string()).collect();

    chunks.into_iter().map(|chunk| {
        let embedding = embed::embed_text(&chunk);
        let meta = serde_json::json!({"text": chunk}).to_string();
        (embedding, meta)
    }).collect()
}

async fn upload_resume(data: web::Json<UploadRequest>, db: web::Data<std::sync::Mutex<KDTreeVectorDB>>) -> Result<String> {
    let vecs = vectorize_resume(&data.resume);
    let mut db = db.lock().unwrap();
    for (vec, meta) in vecs {
        db.insert(vec, meta);
    }
    Ok("Resume vectorized and inserted".to_string())
}

async fn query(data: web::Json<QueryRequest>, db: web::Data<std::sync::Mutex<KDTreeVectorDB>>) -> Result<String> {
    let query_vec = embed::embed_text(&data.query);
    let db = db.lock().unwrap();
    let results = db.query(&query_vec, data.k);

    let mut resp: Vec<QueryResult> = Vec::new();
    for (id, dist) in results {
        let meta_str = db.get_metadata(id).unwrap_or_default();
        let metadata: serde_json::Value = serde_json::from_str(&meta_str).unwrap_or(serde_json::Value::Null);
        resp.push(QueryResult { id, distance: dist, metadata });
    }
    Ok(serde_json::to_string(&resp).unwrap())
}

async fn clear_database(db: web::Data<std::sync::Mutex<KDTreeVectorDB>>) -> Result<String> {
    let mut db = db.lock().unwrap();
    db.clear();
    Ok("Database cleared successfully".to_string())
}

async fn get_stats(db: web::Data<std::sync::Mutex<KDTreeVectorDB>>) -> Result<String> {
    let db = db.lock().unwrap();
    let stats = serde_json::json!({
        "total_vectors": db.size(),
        "dimension": 384
    });
    Ok(serde_json::to_string(&stats).unwrap())
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    embed::init_embeddings("sentence-transformers/all-MiniLM-L6-v2");
    let db = web::Data::new(std::sync::Mutex::new(KDTreeVectorDB::new(384)));

    HttpServer::new(move || {
        let cors = Cors::permissive();
        
        App::new()
            .wrap(cors)
            .app_data(db.clone())
            .route("/upload_resume", web::post().to(upload_resume))
            .route("/query", web::post().to(query))
            .route("/clear", web::post().to(clear_database))
            .route("/stats", web::get().to(get_stats))
    })
    .bind("0.0.0.0:8080")?
    .run()
    .await
}
