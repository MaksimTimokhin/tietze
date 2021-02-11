#pragma once


#include <memory>
#include <stdexcept>
#include <string>

#include <utils/trie.hpp>
#include <presentation/presentation.hpp>

class Parser {
public:
  Parser() = default;
  std::unique_ptr<GroupPresentation> ParseFromString(const std::string &presentation) {
    presentation_ = presentation;

    delimiter_pos_ = presentation_.find('|');
    if (presentation_.front() != '<' || presentation_.back() != '>' ||
        delimiter_pos_ == std::string::npos) {
      throw std::runtime_error("Bad syntax");
    }

    ParseGenerators();
    ParseRelations();

    return std::make_unique<GroupPresentation>(generators_, relations_);
  }

private:
  void ParseGenerators() {
    generators_.clear();
    generator_trie_.reset(nullptr);

    std::string curr_name;
    size_t curr_index = 1;
    TrieBuilder trie_builder;
    for (size_t i = 1; presentation_[i] != '|'; ++i) {
      switch (presentation_[i]) {
      case ' ': {
        continue;
      }
      case ',': {
        generators_.push_back(curr_name);
        trie_builder.Add(curr_name, curr_index++);
        curr_name.clear();
        break;
      }
      default: {
        curr_name.push_back(presentation_[i]);
      }
      }
    }
    generators_.push_back(curr_name);
    trie_builder.Add(curr_name, curr_index++);

    generator_trie_ = trie_builder.Build();
  }

  void ParseRelations() {
    relations_.assign(1, {});

    std::string curr_relation;
    size_t curr_index = 0;
    NodeReference curr_node = generator_trie_->Root();
    for (size_t i = delimiter_pos_ + 1; i < presentation_.size() - 1; ++i) {
      switch (presentation_[i]) {
      case ' ': {
        continue;
      }
      case '-': {
        continue;
      }
      case ',': {
        ++curr_index;
        relations_.resize(relations_.size() + 1);
        break;
      }
      default: {
        curr_node = curr_node.Next(presentation_[i]);
        if (!curr_node) {
          throw std::runtime_error("Bad relation");
        }

        curr_node.GenerateMatches(
            [this, curr_index, i, &curr_node](size_t variable_index) {
              if (presentation_[i + 1] == '-') {
                relations_[curr_index].push_back(-variable_index);
              } else {
                relations_[curr_index].push_back(variable_index);
              }
              curr_node = generator_trie_->Root();
            });
      }
      }
    }
  }

  std::string presentation_;
  size_t delimiter_pos_;

  std::unique_ptr<Trie> generator_trie_{nullptr};

  std::vector<std::string> generators_;
  std::vector<std::vector<int>> relations_;
};