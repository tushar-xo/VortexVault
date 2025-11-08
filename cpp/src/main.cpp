#include "crow_all.h"
#include "KDTreeVectorDB.h"
#include "embed.h"
#include <regex>
#include "json.hpp"
#include <memory>

static std::shared_ptr<KDTreeVectorDB> db = std::make_shared<KDTreeVectorDB>(384);

std::vector<std::pair<std::vector<float>, std::string>> vectorizeResume(const std::string& resume_text) {
    std::vector<std::string> chunks;
    std::regex sentence_regex(R"([^\.!\?]+[\.!\?]+)");
    auto begin = std::sregex_iterator(resume_text.begin(), resume_text.end(), sentence_regex);
    auto end = std::sregex_iterator();
    for (std::sregex_iterator i = begin; i != end; ++i) {
        chunks.push_back(i->str());
    }

    std::vector<std::pair<std::vector<float>, std::string>> vecs;
    for (const auto& chunk : chunks) {
        auto embedding = embedText(chunk);
        nlohmann::json meta = {{"text", chunk}};
        vecs.emplace_back(embedding, meta.dump());
    }
    return vecs;
}

int main() {
    crow::SimpleApp app;

    initEmbeddings("path/to/all-MiniLM-L6-v2.gguf"); // Adjust path

    CROW_ROUTE(app, "/upload_resume").methods("POST"_method)
    ([](const crow::request& req) {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");

        std::string resume_text = json["resume"].s();
        auto vecs = vectorizeResume(resume_text);
        for (const auto& [vec, meta] : vecs) {
            db->insert(vec, meta);
        }
        return crow::response(200, "Resume vectorized and inserted");
    });

    CROW_ROUTE(app, "/query").methods("POST"_method)
    ([](const crow::request& req) {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");

        std::string query_text = json["query"].s();
        size_t k = json["k"].i();
        auto query_vec = embedText(query_text);
        auto results = db->query(query_vec, k);

        nlohmann::json resp;
        for (const auto& [id, dist] : results) {
            nlohmann::json item;
            item["id"] = id;
            item["distance"] = dist;
            item["metadata"] = nlohmann::json::parse(db->getMetadata(id));
            resp.push_back(item);
        }
        return crow::response(200, resp.dump());
    });

    app.port(8080).multithreaded().run();

    freeEmbeddings();
    return 0;
}
