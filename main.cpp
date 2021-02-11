#include <fstream>
#include <iostream>
#include <presentation/presentation.hpp>
#include <parser/presentation_parser.hpp>
#include <tietze/transform.hpp>
#include <chrono>

std::unique_ptr<GroupPresentation> ParseGAP(std::istream &in) {
    std::string s;
    std::getline(in, s);
    std::getline(in, s);

    
    std::vector<std::string> generators;
    std::unordered_map<std::string, size_t> name_to_idx;
    for (auto it = std::find(s.begin(), s.end(), '"'), jt = std::find(it + 1, s.end(), '"');
         jt != s.end();
         it = std::find(jt + 1, s.end(), '"'), jt = std::find(it + 1, s.end(), '"')) {
        std::string name = s.substr(it + 1 - s.begin(), jt - it - 1);
        generators.push_back(name);
        name_to_idx[name] = generators.size();
        //std::cout << generators.back() << std::endl;
    }

    std::getline(in, s);
    //std::getline(in, s);
    
    //std::cout << "h\n" << s << std::endl;
    std::vector<std::vector<int>> relators;
    for (auto curr_coma = std::find(s.begin(), s.end(), '['); curr_coma != s.end(); curr_coma = std::find(curr_coma + 1, s.end(), ',')) {
        std::vector<int> relator;

        auto qu = curr_coma + 2;
        while (true) {
            if (*qu == '(') {
                auto name_end = std::find(qu, s.end(), ')');
                std::string name = s.substr(qu + 1 - s.begin(), name_end - qu - 1);
                auto pow_end = std::find(name_end + 1, s.end(), ')');
                int pow = std::stoi(s.substr(name_end + 3 - s.begin(), pow_end - name_end - 3));
                bool reversed = pow < 0;
                if (pow < 0) {
                    pow = -pow;
                }
                for (int i = 0; i < pow; ++i) {
                    relator.push_back(reversed ? -name_to_idx[name] : name_to_idx[name]);
                }
                if (*(pow_end + 1) == ',' || *(pow_end + 1) == ' ') {
                    break;
                } else {
                    qu = pow_end + 2;
                }
            } else {
                auto name_end = std::find_if(qu, s.end(), [] (char c) { return c == '*' || c == ',' || c == ' '; });
                std::string name = s.substr(qu - s.begin(), name_end - qu);
                relator.push_back(name_to_idx[name]);
                if (*name_end == ',' || *name_end == ' ') {
                    break;
                } else {
                    qu = name_end + 1;
                }
            }
        }
        // std::cout << 'h';
        relators.push_back(relator);
        // for (auto t : relator) {
        //     std::cout << t << ' ';
        // }
        // std::cout << std::endl;

        // std::cout << relators.size() << ' ' << qu - s.begin() << '/' << s.size() << std::endl;
    }

    return std::make_unique<GroupPresentation>(generators, relators);
}

void PrintPresentationGap(GroupPresentation &pres, std::ostream &out) {
    out << "local f, g;\nf := FreeGroup( \"";
    size_t gen_cnt = pres.GeneratorCount();
    size_t i = 0;
    for (const auto &[_, gen] : pres.Generators()) {
        out << gen << '"';
        if (++i < gen_cnt) {
            out << ",";
        }
        out << " ";
    }
    out << ");\ng := f / [ ";
    i = 0;
    for (auto &[_, rel] : pres.Relators()) {
        size_t j = 0;
        for (int t: rel.Generators()) {
            out << pres.GetGeneratorByIndex(t);
            if (t < 0) {
                out << "^(-1)";
            }
            if (++j < rel.Size()) {
                out << "*";
            }
        }
        if (++i < pres.RelatorCount()) {
            out << ",";
        }
        out << " ";
    }
    out << "];\nreturn g;";
}

int main(int argc, char **argv) {
    std::string presentation;
    //std::getline(std::cin, presentation);
    std::ifstream f("/home/epicurus/learning/itmo/projects/tietze/in2.txt");
    auto pres = ParseGAP(f);
    std::cout << "presentation with " << pres->GeneratorCount() << " generators and " << pres->RelatorCount() << " relators of total length " << pres->RelatorsTotalLength() << std::endl;
    

    // Parser parser;
    // auto pres = parser.ParseFromString(presentation);


    auto start = std::chrono::high_resolution_clock::now();
    Simplify(*pres, 5, 10, 1000);
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << " milliseconds\n";

    // std::cout << *pres;
    std::ofstream out("out.txt");
    PrintPresentationGap(*pres, out);

    return 0;
}
// <x01,x2,x3,x4,x5,x7,x8,x18|x01x01,x8x8,x3x3,x7x7,x18x18,x4x4,x7x2x3,x4x2-x01,x8x4x2,x3x5x2,x5x01x2,x3x01x2-,x2-x2-x2-,x3x2-x4,x01x3x2,x01x7x2-,x2x2x2,x2x01x3,x3x2x01,x01x2-x3,x18x01x2-,x4x01x2,x2-x5x3,x2-x01x4,x2-x3x01,x2x2x2,x01x7x2-,x01x8x2,x01x8x2,x2x01x2x01,x3x2-x01x2,x2x01x2x01,x3x01x3x5-,x2-x2-x01x3,x2-x3x2-x3,x3x01x3x01x3x01>
// <a,b,s,t,u,v|aa,bbb,ababababab,ss,tt,uu,vv,s-t-st,u-v-uv,s-u-su,s-v-sv,t-u-tu,t-v-tv,a-sau-,a-tav-,a-uas-,a-vat-,b-sbv-t-,b-tbv-u-t-s-,b-ubv-u-,b-vbu->