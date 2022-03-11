#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <pybind11_json/pybind11_json.hpp>
#include <string>
#include <unordered_map>

using namespace pybind11::literals;
using std::string;
using json = nlohmann::json;
using fmt::format;

namespace fs = std::filesystem;
namespace py = pybind11;

string readstr(const fs::path &path)
{
    std::ifstream ifs(path);
    if (!ifs) throw std::runtime_error(format("Can not open {}", path.string()));

    auto size = fs::file_size(path);
    string buf;
    buf.resize(size);

    ifs.read(buf.data(), size);
    return buf;
}

json parse_recursive(const string &tson, std::unordered_map<string, json> &cache, int ref_depth,
                     const fs::path &cwd)
{
    // circle reference detection ?
    if (ref_depth > 16) throw std::runtime_error("Max recursion depth occurred\n");
    return json::parse(
        tson,
        [&](int depth, json::parse_event_t event, json &parsed) {
            if (event != json::parse_event_t::value || !parsed.is_string()) return true;
            const auto &val = parsed.get<string>();
            if (val[0] != '@') return true;
            const auto &pstr = val.substr(1);
            if (cache.find(pstr) == cache.end()) {
                cache[pstr] =
                    parse_recursive(readstr(cwd / fs::path(pstr)), cache, ref_depth + 1, cwd);
            }
            parsed = cache[pstr];

            return true;
        },
        true, true);
}

void unpack_recursive(json &parent)
{
    if (parent.is_object()) {
        while (true) {  // it might be a chained unpack
            // TODO : implement multiple, prioritized unpack
            json::iterator to_unpack = parent.find("*");
            if (to_unpack == parent.end()) break;
            if (!to_unpack.value().is_object()) throw std::runtime_error("Can only unpack object");
            auto to_add = std::move(to_unpack.value());
            parent.erase(to_unpack);

            for (auto &i : to_add.items())  // add without override parent items
                if (parent.find(i.key()) == parent.end()) parent[i.key()] = std::move(i.value());
            for (auto &e : parent)
                if (e.is_structured()) unpack_recursive(e);
        }
    }
    for (auto &e : parent)
        if (e.is_structured()) unpack_recursive(e);
}

json from_text(const string &tson_str, const string &cwd = ".")
{
    std::unordered_map<string, json> cache;
    auto j = parse_recursive(tson_str, cache, 0, fs::path(cwd));
    unpack_recursive(j);
    return j;
}

json from_file(const string &tson_path, const string &cwd = ".")
{
    return from_text(readstr(fs::path(tson_path)), cwd);
}


PYBIND11_MODULE(tson, m)
{
    m.doc() = "tson doc";
    m.def("from_text", &from_text, "parse tson from text", "tson_str"_a, "cwd"_a = ".");
    m.def("from_file", &from_file, "parse tson from file", "tson_path"_a, "cwd"_a = ".");
}