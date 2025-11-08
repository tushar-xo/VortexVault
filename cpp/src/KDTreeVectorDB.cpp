#include "KDTreeVectorDB.h"
#include <mutex>  // Add mutex for thread safety

KDTreeVectorDB::KDTreeVectorDB(int dim) : dimension(dim) {}

float KDTreeVectorDB::distance(const std::vector<float>& a, const std::vector<float>& b) const {
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        sum += std::pow(a[i] - b[i], 2);
    }
    return std::sqrt(sum);
}

std::unique_ptr<KDNode> KDTreeVectorDB::buildTree(std::vector<std::tuple<std::vector<float>, size_t, std::string>>& points, int depth) {
    if (points.empty()) return nullptr;

    int axis = depth % dimension;
    std::sort(points.begin(), points.end(), [axis](const auto& a, const auto& b) {
        return std::get<0>(a)[axis] < std::get<0>(b)[axis];
    });

    size_t median = points.size() / 2;
    auto node = std::make_unique<KDNode>(std::get<0>(points[median]), std::get<1>(points[median]), std::get<2>(points[median]));

    auto left_points = std::vector<std::tuple<std::vector<float>, size_t, std::string>>(points.begin(), points.begin() + median);
    node->left = buildTree(left_points, depth + 1);

    auto right_points = std::vector<std::tuple<std::vector<float>, size_t, std::string>>(points.begin() + median + 1, points.end());
    node->right = buildTree(right_points, depth + 1);

    return node;
}

void KDTreeVectorDB::insertRec(KDNode* node, const std::vector<float>& point, size_t id, const std::string& meta, int depth) {
    int axis = depth % dimension;
    if (point[axis] < node->point[axis]) {
        if (node->left) {
            insertRec(node->left.get(), point, id, meta, depth + 1);
        } else {
            node->left = std::make_unique<KDNode>(point, id, meta);
        }
    } else {
        if (node->right) {
            insertRec(node->right.get(), point, id, meta, depth + 1);
        } else {
            node->right = std::make_unique<KDNode>(point, id, meta);
        }
    }
}

size_t KDTreeVectorDB::insert(const std::vector<float>& vec, const std::string& meta) {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    if (vec.size() != static_cast<size_t>(dimension)) {
        throw std::invalid_argument("Dimension mismatch");
    }
    size_t id = next_id++;
    all_vectors.push_back(vec);
    if (!root) {
        root = std::make_unique<KDNode>(vec, id, meta);
    } else {
        insertRec(root.get(), vec, id, meta, 0);
    }
    return id;
}

void KDTreeVectorDB::batchInsert(const std::vector<std::vector<float>>& vecs, const std::vector<std::string>& metas) {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    std::vector<std::tuple<std::vector<float>, size_t, std::string>> points;
    for (size_t i = 0; i < vecs.size(); ++i) {
        const auto& vec = vecs[i];
        std::string meta = (i < metas.size()) ? metas[i] : "";
        if (vec.size() != static_cast<size_t>(dimension)) {
            throw std::invalid_argument("Dimension mismatch");
        }
        size_t id = next_id++;
        all_vectors.push_back(vec);
        points.emplace_back(vec, id, meta);
    }
    auto existing_points = collectPoints();
    existing_points.insert(existing_points.end(), points.begin(), points.end());
    root = buildTree(existing_points);
}

void KDTreeVectorDB::knnSearch(const KDNode* node, const std::vector<float>& query, int depth, size_t k, std::priority_queue<DistPair>& pq) const {
    if (!node) return;

    float dist = distance(node->point, query);
    if (pq.size() < k) {
        pq.emplace(dist, node->id);
    } else if (dist < pq.top().first) {
        pq.pop();
        pq.emplace(dist, node->id);
    }

    int axis = depth % dimension;
    const auto* next_branch = (query[axis] < node->point[axis]) ? node->left.get() : node->right.get();
    const auto* other_branch = (next_branch == node->left.get()) ? node->right.get() : node->left.get();

    knnSearch(next_branch, query, depth + 1, k, pq);

    float axis_diff = std::abs(query[axis] - node->point[axis]);
    if (pq.size() < k || axis_diff <= pq.top().first) {
        knnSearch(other_branch, query, depth + 1, k, pq);
    }
}

std::vector<std::pair<size_t, float>> KDTreeVectorDB::query(const std::vector<float>& query_vec, size_t k) const {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    if (!root) return {};
    if (query_vec.size() != static_cast<size_t>(dimension)) {
        throw std::invalid_argument("Query dimension mismatch");
    }
    std::priority_queue<DistPair> pq;

    knnSearch(root.get(), query_vec, 0, k, pq);

    std::vector<std::pair<size_t, float>> results;
    while (!pq.empty()) {
        auto [dist, id] = pq.top();
        results.emplace_back(id, dist);
        pq.pop();
    }
    std::reverse(results.begin(), results.end());
    return results;
}

std::vector<std::vector<std::pair<size_t, float>>> KDTreeVectorDB::batchQuery(const std::vector<std::vector<float>>& queries, size_t k) const {
    std::vector<std::vector<std::pair<size_t, float>>> results;
    for (const auto& q : queries) {
        results.push_back(query(q, k));
    }
    return results;
}

KDNode* KDTreeVectorDB::findNode(KDNode* node, size_t id, int depth) {
    if (!node) return nullptr;
    if (node->id == id) return node;

    int axis = depth % dimension;
    auto* branch = (all_vectors[id][axis] < node->point[axis]) ? node->left.get() : node->right.get();
    return findNode(branch, id, depth + 1);
}

std::string KDTreeVectorDB::getMetadata(size_t id) const {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    if (id >= next_id) return "";
    KDNode* node = const_cast<KDTreeVectorDB*>(this)->findNode(root.get(), id, 0);
    return node ? node->metadata : "";
}

KDNode* KDTreeVectorDB::findMin(KDNode* node, int axis, int depth) {
    if (!node) return nullptr;

    int curr_axis = depth % dimension;
    if (curr_axis == axis) {
        if (node->left) return findMin(node->left.get(), axis, depth + 1);
        return node;
    }

    auto* left_min = findMin(node->left.get(), axis, depth + 1);
    auto* right_min = findMin(node->right.get(), axis, depth + 1);
    KDNode* min_node = node;

    if (left_min && left_min->point[axis] < min_node->point[axis]) min_node = left_min;
    if (right_min && right_min->point[axis] < min_node->point[axis]) min_node = right_min;
    return min_node;
}

std::unique_ptr<KDNode> KDTreeVectorDB::deleteRec(std::unique_ptr<KDNode> node, size_t id, int depth) {
    if (!node) return nullptr;

    int axis = depth % dimension;
    if (id == node->id) {
        if (!node->right) return std::move(node->left);
        if (!node->left) return std::move(node->right);
        KDNode* min_node = findMin(node->right.get(), axis, depth + 1);
        node->point = min_node->point;
        node->id = min_node->id;
        node->metadata = min_node->metadata;
        node->right = deleteRec(std::move(node->right), min_node->id, depth + 1);
        return node;
    }

    if (all_vectors[id][axis] < node->point[axis]) {
        node->left = deleteRec(std::move(node->left), id, depth + 1);
    } else {
        node->right = deleteRec(std::move(node->right), id, depth + 1);
    }
    return node;
}

void KDTreeVectorDB::remove(size_t id) {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    if (id >= next_id) throw std::invalid_argument("Invalid ID");
    root = deleteRec(std::move(root), id, 0);
    std::swap(all_vectors[id], all_vectors.back());
    all_vectors.pop_back();
    next_id--;
}

void KDTreeVectorDB::saveRec(std::ofstream& ofs, const KDNode* node) const {
    if (!node) {
        ofs.write(reinterpret_cast<const char*>(&node), sizeof(node));
        return;
    }
    size_t pt_size = node->point.size();
    ofs.write(reinterpret_cast<const char*>(&pt_size), sizeof(pt_size));
    ofs.write(reinterpret_cast<const char*>(node->point.data()), pt_size * sizeof(float));
    ofs.write(reinterpret_cast<const char*>(&node->id), sizeof(node->id));
    size_t meta_size = node->metadata.size();
    ofs.write(reinterpret_cast<const char*>(&meta_size), sizeof(meta_size));
    ofs.write(node->metadata.data(), meta_size);
    saveRec(ofs, node->left.get());
    saveRec(ofs, node->right.get());
}

std::unique_ptr<KDNode> KDTreeVectorDB::loadRec(std::ifstream& ifs) {
    size_t pt_size;
    ifs.read(reinterpret_cast<char*>(&pt_size), sizeof(pt_size));
    if (ifs.eof()) return nullptr;
    std::vector<float> point(pt_size);
    ifs.read(reinterpret_cast<char*>(point.data()), pt_size * sizeof(float));
    size_t id;
    ifs.read(reinterpret_cast<char*>(&id), sizeof(id));
    size_t meta_size;
    ifs.read(reinterpret_cast<char*>(&meta_size), sizeof(meta_size));
    std::string meta(meta_size, '\0');
    ifs.read(&meta[0], meta_size);
    auto node = std::make_unique<KDNode>(point, id, meta);
    node->left = loadRec(ifs);
    node->right = loadRec(ifs);
    return node;
}

void KDTreeVectorDB::saveToFile(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) throw std::runtime_error("File open failed");
    ofs.write(reinterpret_cast<const char*>(&dimension), sizeof(dimension));
    ofs.write(reinterpret_cast<const char*>(&next_id), sizeof(next_id));
    size_t vec_count = all_vectors.size();
    ofs.write(reinterpret_cast<const char*>(&vec_count), sizeof(vec_count));
    for (const auto& vec : all_vectors) {
        ofs.write(reinterpret_cast<const char*>(vec.data()), dimension * sizeof(float));
    }
    saveRec(ofs, root.get());
}

void KDTreeVectorDB::loadFromFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) throw std::runtime_error("File open failed");
    ifs.read(reinterpret_cast<char*>(&dimension), sizeof(dimension));
    ifs.read(reinterpret_cast<char*>(&next_id), sizeof(next_id));
    size_t vec_count;
    ifs.read(reinterpret_cast<char*>(&vec_count), sizeof(vec_count));
    all_vectors.resize(vec_count);
    for (auto& vec : all_vectors) {
        vec.resize(dimension);
        ifs.read(reinterpret_cast<char*>(vec.data()), dimension * sizeof(float));
    }
    root = loadRec(ifs);
}

std::vector<std::tuple<std::vector<float>, size_t, std::string>> KDTreeVectorDB::collectPoints() const {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    std::vector<std::tuple<std::vector<float>, size_t, std::string>> points;
    collectRec(root.get(), points);
    return points;
}

void KDTreeVectorDB::collectRec(const KDNode* node, std::vector<std::tuple<std::vector<float>, size_t, std::string>>& points) const {
    if (!node) return;
    points.emplace_back(node->point, node->id, node->metadata);
    collectRec(node->left.get(), points);
    collectRec(node->right.get(), points);
}

size_t KDTreeVectorDB::size() const {
    std::lock_guard<std::mutex> lock(db_mutex);  // Add thread safety
    return next_id;
}
