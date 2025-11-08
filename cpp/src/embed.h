#ifndef EMBED_H
#define EMBED_H

#include <cstddef>
#include <string>
#include <vector>

#ifndef EMBEDDING_DIM
constexpr std::size_t EMBEDDING_DIM = 384;
#endif

void initEmbeddings(const std::string& model_path);
std::vector<float> embedText(const std::string& text);
void freeEmbeddings();

#endif // EMBED_H
