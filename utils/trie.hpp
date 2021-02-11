#pragma once

#include <utils/iterators.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

struct TrieNode {
  std::vector<size_t> terminated_string_ids;
  std::map<char, TrieNode> trie_transitions;
};

TrieNode *GetTrieTransition(TrieNode *node, char character) {
  auto it = node->trie_transitions.find(character);
  if (it != node->trie_transitions.end()) {
    return &it->second;
  }
  return nullptr;
}

class NodeReference {
public:
  NodeReference() : node_(nullptr), root_(nullptr) {}

  NodeReference(TrieNode *node, TrieNode *root) : node_(node), root_(root) {}

  NodeReference Next(char character) const {
    return NodeReference(GetTrieTransition(node_, character), root_);
  }

  template <class Callback> void GenerateMatches(Callback on_match) const {
    for (auto &id : TerminatedStringIds()) {
      on_match(id);
    }
  }

  explicit operator bool() const { return node_ != nullptr; }

  bool operator==(NodeReference other) const {
    return node_ == other.node_ && root_ == other.root_;
  }

private:
  using TerminatedStringIterator = std::vector<size_t>::const_iterator;
  using TerminatedStringIteratorRange = IteratorRange<TerminatedStringIterator>;

  TerminatedStringIteratorRange TerminatedStringIds() const {
    return NodeReference::TerminatedStringIteratorRange(
        node_->terminated_string_ids.begin(),
        node_->terminated_string_ids.end());
  }

  TrieNode *node_;
  TrieNode *root_;
};

class Trie {
public:
  Trie() = default;

  Trie(const Trie &) = delete;

  Trie &operator=(const Trie &) = delete;

  NodeReference Root() { return NodeReference(&root_, &root_); }

private:
  TrieNode root_;
  friend class TrieBuilder;
};

class TrieBuilder {
public:
  void Add(const std::string &string, size_t id) {
    words_.push_back(string);
    ids_.push_back(id);
  }

  std::unique_ptr<Trie> Build() {
    auto trie = std::make_unique<Trie>();
    BuildTrie(words_, ids_, trie.get());

    return trie;
  }

private:
  static void BuildTrie(const std::vector<std::string> &words,
                        const std::vector<size_t> &ids, Trie *trie) {
    for (size_t i = 0; i < words.size(); ++i) {
      AddString(&trie->root_, ids[i], words[i]);
    }
  }

  static void AddString(TrieNode *root, size_t string_id,
                        const std::string &string) {
    TrieNode *current_node = root;
    for (char character : string) {
      current_node = &current_node->trie_transitions[character];
    }
    current_node->terminated_string_ids.push_back(string_id);
  }

  std::vector<std::string> words_;
  std::vector<size_t> ids_;
};