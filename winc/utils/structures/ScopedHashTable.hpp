#include <ranges>
#include <optional>
#include <unordered_map>
#include <vector>

template <typename K, typename V>
class ScopedHashTable
{
public:
    ScopedHashTable() = default;

    std::optional<V> lookup(auto&& key)
    {
        for (auto& scope : std::views::reverse(scopes_)) {
            if (auto it = scope.find(key); it != scope.end()) {
                return it->second;
            }
        }

        return std::nullopt;
    }

    void push_scope()
    {
        scopes_.emplace_back();
    }

    void pop_scope()
    {
        assert(scopes_.size() > 0);
        scopes_.pop_back();
    }

    void emplace(auto&&... args)
    {
        assert(!scopes_.empty());
        scopes_.back().emplace(std::forward<decltype(args)>(args)...);
    }

private:
    std::vector<std::unordered_map<K, V>> scopes_;
};
