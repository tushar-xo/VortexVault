#include "embed.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string_view>

namespace {
    bool g_model_initialised = false;
    constexpr float PRIME_SCALE = 1.0f / 251.0f;
}

void initEmbeddings(const std::string& /*model_path*/) {
    g_model_initialised = true;
}

std::vector<float> embedText(const std::string& text) {
    if (!g_model_initialised) {
        throw std::runtime_error("Embeddings requested before initialisation");
    }

    std::vector<float> embedding(EMBEDDING_DIM, 0.0f);
    if (text.empty()) {
        return embedding;
    }

    std::string_view view{text};
    std::hash<std::string_view> hasher;
    std::size_t seed = hasher(view);

    for (std::size_t i = 0; i < EMBEDDING_DIM; ++i) {
        seed ^= (static_cast<std::size_t>(view[i % view.size()]) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
        float normalised = static_cast<float>((seed % 251u)) * PRIME_SCALE;
        embedding[i] = std::sin(normalised * static_cast<float>(i + 1));
    }

    float norm = std::sqrt(std::inner_product(embedding.begin(), embedding.end(), embedding.begin(), 0.0f));
    if (norm > 0.0f) {
        std::for_each(embedding.begin(), embedding.end(), [norm](float& value) {
            value /= norm;
        });
    }

    return embedding;
}

void freeEmbeddings() {
    g_model_initialised = false;
}
