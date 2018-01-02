#include <iostream>
#include <fstream>
#include <locale>
#include <array>
#include <set>
#include <unordered_map>

using std::cout;
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

struct next_word {
  std::wstring word;
  std::set<wchar_t> punct;
};

int main(int argc, char* argv[]) {
  if (argc==1) {
    cout << "usage: " << argv[0] << " input_file ..." << endl;
    return 1;
  }

  std::wstring word;
  std::array<std::wstring,2> key;
  std::unordered_map<decltype(key),next_word,tuple_hash> map;

  std::locale::global(std::locale(""));
  for (int i=1; i<argc; ++i) {
    std::wifstream f(argv[i]);

    for (wchar_t c; f.get(c); ) {
      if (std::isspace(c)) {
        if (!std::get<0>(key).empty() && !std::get<1>(key).empty()) {
          size_t p = word.size();
          for (; p; ) if (!std::ispunct(word[--p])) break;
          ++p;
          auto& next = map[key];
          next.word  = word.substr(0,p);
          for (size_t i=p; i<word.size(); ++i)
            next.punct.insert(word[p]);
        }
        std::get<0>(key) = std::get<1>(key);
        std::get<1>(key) = word;
        word.clear();
      } else {
        word += std::tolower(c);
      }
    } // c
  } // i

  for (const auto& x : map) {
    std::wcout << "(" << x.first[0] << ',' << x.first[1] << ") = "
      << x.second.word
      << endl;
  }
}
