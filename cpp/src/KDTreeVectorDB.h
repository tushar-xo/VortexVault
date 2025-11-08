#ifndef KDTREE_VECTOR_DB_H
#define KDTREE_VECTOR_DB_H

#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <utility>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <string>
#include "json.hpp"
#include <mutex>

struct KDNode {
    std::vector<float> point;
    size_t id;
    std::string metadata; // JSON string
    std::unique_ptr<KDNode> left;
    std::unique_ptr<KDNode> right;

    KDNode(const std::vector<float>& p, size_t i, const std::string& meta) : point(p), id(i), metadata(meta) {}
};

class KDTreeVectorDB {
private:
    int dimension;
    std::unique_ptr<KDNode> root;
    size_t next_id = 0;
    std::vector<std::vector<float>> all_vectors;
    mutable std::mutex db_mutex;

    float distance(const std::vector<float>& a, const std::vector<float>& b) const;
    std::unique_ptr<KDNode> buildTree(std::vector<std::tuple<std::vector<float>, size_t, std::string>>& points, int depth = 0);
    void insertRec(KDNode* node, const std::vector<float>& point, size_t id, const std::string& meta, int depth);
    using DistPair = std::pair<float, size_t>;
    void knnSearch(const KDNode* node, const std::vector<float>& query, int depth, size_t k, std::priority_queue<DistPair>& pq) const;
    KDNode* findNode(KDNode* node, size_t id, int depth);
    KDNode* findMin(KDNode* node, int axis, int depth);
    std::unique_ptr<KDNode> deleteRec(std::unique_ptr<KDNode> node, size_t id, int depth);
    void saveRec(std::ofstream& ofs, const KDNode* node) const;
    std::unique_ptr<KDNode> loadRec(std::ifstream& ifs);
    std::vector<std::tuple<std::vector<float>, size_t, std::string>> collectPoints() const;
    void collectRec(const KDNode* node, std::vector<std::tuple<std::vector<float>, size_t, std::string>>& points) const;

public:
    KDTreeVectorDB(int dim);
    size_t insert(const std::vector<float>& vec, const std::string& meta = "");
    void batchInsert(const std::vector<std::vector<float>>& vecs, const std::vector<std::string>& metas);
    void remove(size_t id);
    std::vector<std::pair<size_t, float>> query(const std::vector<float>& query_vec, size_t k) const;
    std::vector<std::vector<std::pair<size_t, float>>> batchQuery(const std::vector<std::vector<float>>& queries, size_t k) const;
    std::string getMetadata(size_t id) const;
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    size_t size() const;
};

#endif // KDTREE_VECTOR_DB_H
