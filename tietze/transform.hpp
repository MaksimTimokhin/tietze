#pragma once

#include <presentation/presentation.hpp>
#include <algorithm>

bool EliminateGenerator(GroupPresentation &pres, size_t expansionLimit = 1s00, std::ostream &info = std::cout) {
    std::unordered_map<size_t, size_t> occurences;
    std::unordered_map<size_t, size_t> subst_idx;


    for (const auto &[rel_id, relator] : pres.Relators()) {
        std::unordered_map<size_t, size_t> rel_occurences;        
        for (size_t i = 0; i < relator.Size(); ++i) {
            int gen = relator.GetGen(i);
            if (gen < 0) {
                gen = -gen;
            }
            ++rel_occurences[gen];
            ++occurences[gen];
        }

        for (const auto &[gen_id, occ] : rel_occurences) {
            if (occ == 1 &&
             (!subst_idx.count(gen_id) || relator.Size() < pres.GetRelatorByIndex(subst_idx[gen_id]).Size())) {
                 subst_idx[gen_id] = rel_id;
            }
        }
    }

    std::vector<size_t> unused_gens;
    for (const auto &[gen_id, _] : pres.Generators()) {
        if (!occurences.count(gen_id)) {
            unused_gens.push_back(gen_id);
        }
    }
    for (size_t gen : unused_gens) {
        pres.RemoveGeneratorByIndex(gen);
    }

    size_t eliminated_gen_id = 0;
    size_t subst_len = 0;
    for (const auto &[gen_id, rel_id] : subst_idx) {
        if ((occurences[gen_id] - 1) * (static_cast<int>(pres.GetRelatorByIndex(rel_id).Size()) - 2) <= expansionLimit &&
            (eliminated_gen_id == 0 ||
             (occurences[gen_id] - 1) * (pres.GetRelatorByIndex(rel_id).Size() - 2) < subst_len)) {
            eliminated_gen_id = gen_id;
            subst_len = (occurences[gen_id] - 1) * (static_cast<int>(pres.GetRelatorByIndex(rel_id).Size()) - 2);
        }
    }

    if (eliminated_gen_id == 0) {
        return false;
    }

    Relator &rel = pres.GetRelatorByIndex(subst_idx[eliminated_gen_id]);
    std::vector<size_t> matches;
    std::vector<int> gen(1, eliminated_gen_id), gen_(1, -eliminated_gen_id);
    std::vector<int> subst;
    subst.reserve(rel.Size() - 1);
    rel.FindSubstring(gen, matches);
    if (!matches.empty()) {
        for (size_t i = 1; i < rel.Size(); ++i) {
            subst.push_back(-rel.GetGen(matches[0] - i));
        }
    } else {
        rel.FindSubstring(gen_, matches);
        for (size_t i = 1; i < rel.Size(); ++i) {
            subst.push_back(rel.GetGen(matches[0] + i));
        }
    }

    info << "eliminating " << pres.GetGeneratorByIndex(eliminated_gen_id) << " = ";
    pres.RemoveGeneratorByIndex(eliminated_gen_id);

    for (auto t : subst) {
        info << pres.GetGeneratorByIndex(t);
        if (t < 0) {
            info << "-";
        }
    }
    info << std::endl;

    std::vector<int> subst_;
    subst_.reserve(subst.size());
    for (int t : IteratorRange<std::vector<int>::reverse_iterator>(subst.rbegin(), subst.rend())) {
        subst_.push_back(-t);
    }

    pres.RemoveRelatorByIndex(subst_idx[eliminated_gen_id]);
    std::vector<size_t> trash;
    for (auto &[rel_id, relator] : pres.Relators()) {
        relator.Substitute(gen, subst);
        relator.Substitute(gen_, subst_);
        if (relator.IsTrivial()) {
            trash.push_back(rel_id);
        }
    }
    for (size_t rel_id : trash) {
        pres.RemoveRelatorByIndex(rel_id);
    }
    return true;
}

void Search(GroupPresentation &pres, bool equal = false, size_t save_limit = 10, size_t search_simultaneous = 20, std::ostream &info = std::cout) {
    size_t start_len = pres.RelatorsTotalLength();
    size_t new_len = start_len;

    // auto out_rel = [&] (size_t i) {
    //     for (int t : pres.GetRelatorByIndex(i).Generators()) {
    //         std::cout << pres.GetGeneratorByIndex(t);
    //         if (t < 0) {
    //             std::cout << '-';
    //         }
    //     }
    // };
    do {
        std::vector<std::pair<int, int>> relators;
        relators.reserve(pres.RelatorCount());
        for (const auto &[id, rel] : pres.Relators()) {
            relators.emplace_back(rel.Size(), id);
        }
        std::sort(relators.begin(), relators.end());

        std::vector<size_t> trash;
        for (size_t i = 0; i < relators.size();) {
            std::cout << i << '/' << relators.size() << std::endl;          
            size_t min_match_len = (relators[i].first + 2 - equal) / 2;

            std::unordered_map<size_t, std::pair<size_t, size_t>> substr;
            for (size_t short_count = 0; i < relators.size() && (relators[i].first + 2 - equal) / 2 == min_match_len && short_count < search_simultaneous; ++i) {
                ++short_count;
                auto &rel = pres.GetRelatorByIndex(relators[i].second);
                for (size_t pos = 0; pos < rel.Size(); ++pos) {
                    size_t hash = rel.SubstringHash(pos, min_match_len);
                    if (!substr.count(hash)) {
                        substr[hash] = std::make_pair(i, pos);
                    }
                }
            }
            if (i >= relators.size()) {
                break;
            }

            for (size_t long_i = i; long_i < relators.size(); ++long_i) {
                auto &long_rel = pres.GetRelatorByIndex(relators[long_i].second);
                if (long_rel.Size() < min_match_len) {
                    continue;
                }

                for (size_t pos = 0; pos < long_rel.Size(); ++pos) {
                    size_t hash = long_rel.SubstringHash(pos, min_match_len);
                    bool reversed = false;
                    auto it = substr.find(hash);
                    if (it == substr.end()) {
                        hash = long_rel.ConjugateSubstringHash(pos, min_match_len);
                        it = substr.find(hash);
                        reversed = true;
                    }
                    if (it == substr.end()) {
                        continue;
                    }
                    auto [short_i, short_pos] = it->second;
                    auto &short_rel = pres.GetRelatorByIndex(relators[short_i].second);
                
                    std::vector<int> word;
                    std::vector<int> complement;
                    word.reserve(min_match_len);
                    if (!reversed) {
                        while(word.size() < short_rel.Size()) {
                            if (short_rel.GetGen(short_pos + word.size()) != long_rel.GetGen(pos + word.size())) {
                                break;
                            }
                            word.push_back(short_rel.GetGen(short_pos + word.size()));
                        }
                        if (word.size() < min_match_len) {
                            continue;
                        }
                        complement.reserve(short_rel.Size() - word.size());
                        // std::cout << short_pos << ' ' << word.size() << ") " << std::endl;;
                        for (int k = 0; k + word.size() < short_rel.Size(); ++k) {
                            // out_rel(relators[short_i].second);
                            // std::cout << std::endl;
                            // std::cout << static_cast<int>(short_pos) - k - 1 << ' ' << pres.GetGeneratorByIndex(short_rel.GetGen(static_cast<int>(short_pos) - k - 1)) << std::endl;
                            complement.push_back(-short_rel.GetGen(static_cast<int>(short_pos) - k - 1));
                        }
                    } else {
                        while(word.size() < short_rel.Size()) {
                            if (-short_rel.GetGen(short_pos + min_match_len - 1 - word.size()) != long_rel.GetGen(pos + word.size())) {
                                break;
                            }
                            word.push_back(long_rel.GetGen(pos + word.size()));
                        }

                        if (word.size() < min_match_len) {
                            continue;
                        }
                        complement.reserve(short_rel.Size() - word.size());
                        //std::cout << short_pos << ' ' << word.size() << ") ";
                        for (size_t k = 0; k + word.size() < short_rel.Size(); ++k) {
                            complement.push_back(short_rel.GetGen(short_pos + word.size() + k));
                        }
                    }

                    std::vector<int> rev_word;
                    rev_word.reserve(word.size());
                    for (size_t x = 0; x < word.size(); ++x) {
                        rev_word.push_back(-word[word.size() - 1 - x]);
                    }
                    std::vector<int> rev_compl;
                    rev_compl.reserve(complement.size());
                    for (size_t x = 0; x < complement.size(); ++x) {
                        rev_compl.push_back(-complement[complement.size() - 1 - x]);
                    }

                    // std::cout << "replacing ";
                    // for (auto t : word) {
                    //   std::cout << pres.GetGeneratorByIndex(t);
                    //   if (t < 0) std::cout << '-';  
                    // }
                    // std::cout << " = ";
                    // for (auto t : complement) {
                    //     std::cout << pres.GetGeneratorByIndex(t);
                    //     if (t < 0) std::cout << '-';  
                    // }
                    // std::cout << " in ";
                    // out_rel(relators[long_i].second);
                    // std::cout << std::endl;
                        
                    long_rel.Substitute(word, complement);
                    long_rel.Substitute(rev_word, rev_compl);
                    if (long_rel.IsTrivial()) {
                        trash.push_back(relators[long_i].second);
                    }
                    break;
                }
            }
        }
        start_len = new_len;
        new_len = pres.RelatorsTotalLength();

        for (size_t rel_id : trash) {
            pres.RemoveRelatorByIndex(rel_id);
        }
    } while (!equal && 100 * start_len > 100 * new_len + start_len * save_limit);
}

void Simplify(GroupPresentation &pres, size_t stages = 20, size_t save_limit = 10, size_t search_simultaneous = 20, std::ostream &info = std::cout) {
    for (size_t i = 0; i < stages; ++i) {
        Search(pres, false, save_limit, search_simultaneous + 20 - i * (stages != 1 ? search_simultaneous / (stages - 1) : 0));
        info << "there are " << pres.GeneratorCount() << " generators and " << pres.RelatorCount() << " relators of total length " << pres.RelatorsTotalLength() << std::endl;
        Search(pres, true, save_limit, search_simultaneous - i * (search_simultaneous / stages));
        info << "there are " << pres.GeneratorCount() << " generators and " << pres.RelatorCount() << " relators of total length " << pres.RelatorsTotalLength() << std::endl;
        while (EliminateGenerator(pres)) {
        }
        info << "there are " << pres.GeneratorCount() << " generators and " << pres.RelatorCount() << " relators of total length " << pres.RelatorsTotalLength() << std::endl;
    }
}