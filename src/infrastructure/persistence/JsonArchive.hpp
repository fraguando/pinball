#pragma once
#include "domain/serialization/IArchive.hpp"

#include <nlohmann/json.hpp>

namespace pinball::infra {

// Writes element fields into a JSON object.
class JsonWriteArchive final : public domain::IArchive {
public:
    explicit JsonWriteArchive(nlohmann::json& j) : j_(j) {}
    bool isLoading() const override { return false; }

    void field(const char* key, float& v) override { j_[key] = v; }
    void field(const char* key, int& v) override { j_[key] = v; }
    void field(const char* key, bool& v) override { j_[key] = v; }
    void field(const char* key, glm::vec3& v) override {
        j_[key] = {v.x, v.y, v.z};
    }
    void field(const char* key, std::vector<glm::vec3>& v) override {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& p : v) arr.push_back({p.x, p.y, p.z});
        j_[key] = std::move(arr);
    }

private:
    nlohmann::json& j_;
};

// Reads element fields from a JSON object (missing keys leave defaults).
class JsonReadArchive final : public domain::IArchive {
public:
    explicit JsonReadArchive(const nlohmann::json& j) : j_(j) {}
    bool isLoading() const override { return true; }

    void field(const char* key, float& v) override {
        if (j_.contains(key)) v = j_[key].get<float>();
    }
    void field(const char* key, int& v) override {
        if (j_.contains(key)) v = j_[key].get<int>();
    }
    void field(const char* key, bool& v) override {
        if (j_.contains(key)) v = j_[key].get<bool>();
    }
    void field(const char* key, glm::vec3& v) override {
        if (j_.contains(key) && j_[key].is_array() && j_[key].size() == 3) {
            v.x = j_[key][0].get<float>();
            v.y = j_[key][1].get<float>();
            v.z = j_[key][2].get<float>();
        }
    }
    void field(const char* key, std::vector<glm::vec3>& v) override {
        if (!j_.contains(key) || !j_[key].is_array()) return;
        v.clear();
        for (const auto& p : j_[key]) {
            if (p.is_array() && p.size() == 3)
                v.emplace_back(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
        }
    }

private:
    const nlohmann::json& j_;
};

} // namespace pinball::infra
