use std::collections::hash_map::DefaultHasher;
use std::hash::{Hash, Hasher};

const EMBEDDING_DIM: usize = 384;
const PRIME_SCALE: f32 = 1.0 / 251.0;

static mut MODEL_INITIALIZED: bool = false;

pub fn init_embeddings(_model_name: &str) {
    unsafe {
        MODEL_INITIALIZED = true;
    }
}

pub fn embed_text(text: &str) -> Vec<f32> {
    unsafe {
        if !MODEL_INITIALIZED {
            panic!("Embeddings not initialized");
        }
    }

    let mut embedding = vec![0.0f32; EMBEDDING_DIM];
    
    if text.is_empty() {
        return embedding;
    }

    let mut hasher = DefaultHasher::new();
    text.hash(&mut hasher);
    let mut seed = hasher.finish();

    for i in 0..EMBEDDING_DIM {
        let char_idx = i % text.len();
        let char_byte = text.as_bytes()[char_idx] as u64;
        seed ^= char_byte.wrapping_add(0x9e3779b97f4a7c15u64)
            .wrapping_add(seed << 6)
            .wrapping_add(seed >> 2);
        
        let normalized = ((seed % 251) as f32) * PRIME_SCALE;
        embedding[i] = (normalized * (i as f32 + 1.0)).sin();
    }

    let norm: f32 = embedding.iter().map(|x| x * x).sum::<f32>().sqrt();
    if norm > 0.0 {
        embedding.iter_mut().for_each(|x| *x /= norm);
    }

    embedding
}
