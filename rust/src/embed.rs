use sentence_transformers::SentenceTransformer;

static mut MODEL: Option<SentenceTransformer> = None;

pub fn init_embeddings(model_name: &str) {
    unsafe {
        MODEL = Some(SentenceTransformer::from_pretrained(model_name).unwrap());
    }
}

pub fn embed_text(text: &str) -> Vec<f32> {
    unsafe {
        MODEL.as_ref().unwrap().encode(text).unwrap()
    }
}
