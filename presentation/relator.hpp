#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <utils/iterators.hpp>
#include <iostream>
#include <stack>

class Relator {
public:
    template <typename Iterator>
    Relator(Iterator first, Iterator last) : relator_(first, last) {
      CalculateHash();
    }

    size_t Size() const {
      return relator_.size();
    }

    size_t MinimalMatchLength() const {
      return (relator_.size() + 2) / 2;
    }

    int GetGen(int pos) const {
      if (pos < 0) {
        pos = (relator_.size() - (-pos) % relator_.size());
      }
      pos %= relator_.size();
      return relator_[pos];
    }

    bool IsTrivial() const {
      std::stack<int> g;
      for (int t : relator_) {
        if (g.empty() || g.top() != -t) {
          g.push(t);
        } else {
          g.pop();
        }
      }
      return g.empty();
    }

    IteratorRange<std::vector<int>::iterator> Generators() {
      return IteratorRange<std::vector<int>::iterator>(relator_.begin(), relator_.end());
    }

    void FindSubstring(const std::vector<int> &s, std::vector<size_t> &matches) const {
      if (relator_.size() == 0) {
        return;
      }

      std::vector<size_t> pi(s.size() + 1);
      for (size_t i = 1; i < s.size(); ++i) {
        size_t j = pi[i - 1];
        while (j > 0 && s[i] != s[j]) {
          j = pi[j - 1];
        }
        if (s[i] == s[j]) {
          ++j;
        }
        pi[i] = j;
      }

      size_t c_pi = 0;
      for (size_t pos = 0; pos + 1 < relator_.size() + s.size(); ++pos) {
        while (c_pi > 0 && (c_pi == s.size() || GetGen(pos) != s[c_pi])) {
          c_pi = pi[c_pi - 1];
        }
        if (GetGen(pos) == s[c_pi]) {
          ++c_pi;
        }
        if (c_pi == s.size()) {
          matches.push_back(pos - s.size() + 1);
        }
      }
    }

    size_t SubstringHash(size_t pos, size_t len) const {
      if (pos + len <= Size()) {
        return hash_[pos + len] - hash_[pos] * deg_[len];
      }
      return hash_[pos + len - Size()] + hash_[Size()] * deg_[pos + len - Size()] - hash_[pos] * deg_[len];
    }

    size_t ConjugateSubstringHash(size_t pos, size_t len) const {
      if (pos + len <= Size()) {
        return conj_hash_[Size() - pos] - conj_hash_[Size() - pos - len] * deg_[len];
      }
      return conj_hash_[Size() - pos] + conj_hash_[Size()] * deg_[Size() - pos] - conj_hash_[2 * Size() - len - pos] * deg_[len];
    }

    void Substitute(const std::vector<int> &from, const std::vector<int> &to) {
      std::vector<size_t> matches;
      FindSubstring(from, matches);
      if (matches.empty()) {
        return;
      }

      std::vector<int> result;
      size_t next_match = 0;
      size_t bad_pref = 0;
      for (size_t i = 0; i < relator_.size(); ++i) {
        if (i == matches[next_match]) {
          std::copy(to.begin(), to.end(), std::back_inserter(result));
          i += from.size() - 1;
          if (i >= relator_.size()) {
            bad_pref = i - relator_.size() + 1;
          }
          while (next_match < matches.size() && matches[next_match] <= i) {
            ++next_match;
          }
          if (next_match == matches.size() || matches[next_match] + from.size() > matches[0] + relator_.size()) {
            next_match = 0;
          }
          continue;
        }
        result.push_back(relator_[i]);
      }
      relator_ = std::vector<int>(result.begin() + bad_pref, result.end());
      CalculateHash();
    }

private:
  void CalculateHash() {
    hash_.assign(relator_.size() + 1, 0);
    conj_hash_.assign(relator_.size() + 1, 0);
    deg_.assign(relator_.size() + 1, 1);
    for (size_t i = 1; i <= relator_.size(); ++i) {
      deg_[i] = deg_[i - 1] * kHashP;
      hash_[i] = hash_[i - 1] * kHashP + relator_[i - 1];
      conj_hash_[i] = conj_hash_[i - 1] * kHashP - relator_[relator_.size() - i];
    }
  }

private:
  std::vector<int> relator_;
  std::vector<size_t> hash_;
  std::vector<size_t> conj_hash_;
  std::vector<size_t> deg_;
  static const size_t kHashP = 1'000'000'007;
};