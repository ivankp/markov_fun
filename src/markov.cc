#include <iostream>
#include <fstream>
#include <locale>
#include <array>
#include <set>
#include <unordered_map>
#include <random>
#include <chrono>

using std::cout;
using std::wcout;
using std::endl;

template <typename F, size_t... I>
void for_(F f, std::index_sequence<I...>) {
  // using expander = int[];
  // (void)expander{0, ((void)f(std::integral_constant<size_t,I>{}), 0)...};
  ( f(std::integral_constant<size_t,I>{}),... );
}
template <size_t N, typename F>
void for_(F f) { for_(f, std::make_index_sequence<N>{}); }

template <typename T>
void hash_combine(size_t& seed, const T& v) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct tuple_hash {
  template <typename T>
  size_t operator()(const T& t) const {
    size_t seed = 0;
    for_<std::tuple_size<T>::value>(
      [&](auto i){
        hash_combine(seed,std::get<i.value>(t));
      }
    );
    return seed;
  }
};

struct word_properties {
  std::set<wchar_t> punct;
  bool cap = true;
};

class select_rand_impl {
  std::mt19937 gen;
public:
  select_rand_impl()
  : gen(std::chrono::system_clock::now().time_since_epoch().count()) { }
  template <typename T>
  const auto& operator()(const T& container) {
    auto it = container.begin();
    std::uniform_int_distribution<unsigned int> dist(0,container.size());
    std::advance(it,dist(gen));
    return *it;
  }
} select_rand;

int main(int argc, char* argv[]) {
  if (argc==1) {
    cout << "usage: " << argv[0] << " input_file ..." << endl;
    return 1;
  }

  std::wstring word;
  std::unordered_map<std::wstring,word_properties> dict;
  std::array<std::wstring,2> key;
  std::unordered_map<
    decltype(key),
    const decltype(dict)::value_type*,
    tuple_hash
  > map;

  std::locale::global(std::locale(""));
  bool cap = true;
  for (int i=1; i<argc; ++i) {
    std::wifstream f(argv[i]);

    for (wchar_t c; f.get(c); ) {
      if (std::isspace(c)) {
        if (!std::get<0>(key).empty() &&
            !std::get<1>(key).empty() &&
            !word.empty()
        ) {
          size_t a = 0, b = word.size();
          for (; b; ) if (!std::ispunct(word[--b])) break;
          for (; a<=b; ++a) if (!std::ispunct(word[a])) break;
          ++b;
          if (a!=b) {
            auto word_no_punct = word.substr(a,b-a);
            auto& dict_word = *dict.try_emplace(word_no_punct).first;
            for (size_t i=b; i<word.size(); ++i)
              dict_word.second.punct.insert(word[i]);
            word = std::move(word_no_punct);
            map[key] = &dict_word;
            if (!cap) dict_word.second.cap = false;
          } else {
            word.clear();
          }
        }
        if (!word.empty()) {
          std::get<0>(key) = std::get<1>(key);
          std::get<1>(key) = word;
          word.clear();
        }
        cap = true;
      } else {
        const auto lc = std::tolower(c);
        if (word.empty() && lc==c) cap = false;
        word += lc;
      }
    } // c
  } // i

  wcout << "dict: " << dict.size() << endl;
  wcout << "map : " << map.size() << endl;

  /*
  for (const auto& x : map) {
    wcout << "(" << x.first[0] << ',' << x.first[1] << ") = "
      << x.second->first << ' ' << x.second->second.cap << ' ';
    for (auto c : x.second->second.punct)
      wcout << c;
    wcout << endl;
  }
  */

  auto& seed = select_rand(map);
  key  = seed.first;
  word = seed.second->first;

  wcout << key[0] <<' '<< key[1] <<' '<< word;
  for (unsigned i=0; i<35; ++i) {
    std::get<0>(key) = std::get<1>(key);
    std::get<1>(key) = word;
    const auto it = map.find(key);
    if (it==map.end()) break;
    word = it->second->first;
    wcout <<' '<< word;
  }
  wcout << endl;
}
