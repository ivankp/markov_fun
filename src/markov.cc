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
using std::get;
using namespace std::string_literals;

template <typename F, size_t... I>
void for_(F f, std::index_sequence<I...>) {
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
        hash_combine(seed,get<i.value>(t));
      }
    );
    return seed;
  }
};

struct word_properties {
  // std::set<wchar_t> punct;
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
    std::uniform_int_distribution<unsigned int> dist(0,container.size()-1);
    std::advance(it,dist(gen));
    return *it;
  }
} select_rand;

std::set<std::wstring> abbrev {{
  L"mr"s, L"mrs"s, L"e.g"s
}};

std::unordered_map<std::wstring,word_properties> dict;
struct dict_ptr {
  using type = decltype(dict)::value_type*;
  type value;
  dict_ptr& operator=(type ptr) noexcept {
    value = ptr;
    return *this;
  }
  auto operator* () const noexcept { return value->first; }
  type operator->() const noexcept { return value; }
  operator bool() const noexcept { return value; }
};
namespace std {
template <> struct hash<dict_ptr> {
  size_t operator()(const dict_ptr& t) const {
    return std::hash<dict_ptr::type>{}(t.value);
  }
};
}
std::basic_ostream<wchar_t>& operator<<(
  std::basic_ostream<wchar_t>& wout, const dict_ptr& ptr
) {
  return wout << *ptr;
}
std::array<dict_ptr,2> key;
std::unordered_map<
  decltype(key),
  std::set<dict_ptr>,
  tuple_hash
> map;

int main(int argc, char* argv[]) {
  if (argc==1) {
    cout << "usage: " << argv[0] << " input_file ..." << endl;
    return 1;
  }

  std::wstring word;
  dict_ptr dict_word;
  std::locale::global(std::locale(""));
  bool cap = true;
  for (int i=1; i<argc; ++i) {
    std::wifstream f(argv[i]);

    for (wchar_t c; f.get(c); ) {
      if (std::isspace(c)) {

        if (word.empty()) continue;
        const auto last = word.back();
        size_t a = 0, b = word.size();
        for (; b; ) if (!std::ispunct(word[--b])) break;
        for (; a<=b; ++a) if (!std::ispunct(word[a])) break;
        ++b;
        word = word.substr(a,b-a);
        if (word.empty()) continue;
        dict_word = &*dict.try_emplace(word).first;

        if (get<0>(key) && get<1>(key)) {
          map[key].insert(dict_word);
          if (!cap) dict_word->second.cap = false;
          if (last=='.' && !abbrev.count(word)) {
            get<0>(key) = nullptr;
            get<1>(key) = nullptr;
          }
        }
        get<0>(key) = get<1>(key);
        get<1>(key) = dict_word;
        word.clear();
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
    wcout << "(" << x.first[0] << ',' << x.first[1] << ") =";
    for (const auto& w : x.second)
      wcout <<' '<< w;
    wcout << endl;
  }
  */

  auto& seed = select_rand(map);
  key = seed.first;
  dict_word = select_rand(seed.second);

  wcout << key[0] <<' '<< key[1] <<' '<< dict_word;
  for (unsigned i=0; i<35; ++i) {
    get<0>(key) = get<1>(key);
    get<1>(key) = dict_word;
    const auto it = map.find(key);
    if (it==map.end()) break;
    dict_word = select_rand(it->second);
    wcout <<' '<< dict_word;
  }
  wcout << endl;
}
