#include <iostream>
#include <fstream>
#include <locale>
#include <array>
#include <set>
#include <unordered_map>
#include <random>
#include <chrono>

#include <boost/lexical_cast.hpp>

using std::cout;
using std::endl;
using std::get;
// using namespace std::string_literals;

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

template <typename T>
const auto& at(const T& container, unsigned i) {
  return *std::next(container.begin(),i);
}

class select_rand_impl {
  std::mt19937 gen;
public:
  select_rand_impl()
  : gen(std::chrono::system_clock::now().time_since_epoch().count()) { }
  template <typename T>
  const auto& operator()(const T& container) {
    std::uniform_int_distribution<unsigned int> dist(0,container.size()-1);
    return at(container,dist(gen));
  }
  template <typename T>
  const auto& operator()(const T& container, unsigned i) {
    std::uniform_int_distribution<unsigned> dist(0,container.size()-1);
    if (i>dist.b()) i = dist(gen);
    cout << i << endl;
    return at(container,i);
  }
} select_rand;

std::set<std::string> abbrev {{
  "Mr", "Mrs", "e.g"
}};

struct word_properties {
  bool period = false;
};

std::unordered_map<std::string,word_properties> dict;
struct dict_ptr {
  using type = decltype(dict)::value_type*;
  type value;
  dict_ptr& operator=(type ptr) noexcept {
    value = ptr;
    return *this;
  }
  bool operator==(dict_ptr x) const noexcept { return value == x.value; }
  bool operator< (dict_ptr x) const noexcept { return value <  x.value; }
  auto& operator*() const noexcept { return value->first; }
  type operator->() const noexcept { return value; }
  operator bool() const noexcept { return value; }
};
namespace std {
template <> struct hash<dict_ptr> {
  size_t operator()(const dict_ptr& ptr) const {
    return std::hash<std::decay_t<decltype(*ptr)>>{}(*ptr);
  }
};
}
std::ostream& operator<<(std::ostream& out, const dict_ptr& ptr) {
  return out << *ptr;
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

  std::string word;
  dict_ptr dict_word;
  for (int i=1; i<argc; ++i) {
    std::ifstream f(argv[i]);

    for (char c; f.get(c); ) {
      if (std::isspace(c)) {
        if (word.empty()) continue;

        bool period = false;
        while (word.back()=='.') {
          word.pop_back();
          period = true;
        }
        if (word.empty()) continue;
        dict_word = &*dict.try_emplace(word).first;
        if (period) dict_word->second.period = true;

        if (get<0>(key) && get<1>(key)) {
          map[key].insert(dict_word);
          // cout <<'*'<< key[0] <<','<< key[1] << ": ";
          // for (const auto& x : map[key]) cout <<' '<< x;
          // cout << endl;
          if (period && !abbrev.count(word)) {
            get<0>(key) = nullptr;
            get<1>(key) = nullptr;
            dict_word   = nullptr;
          }
        }
        get<0>(key) = get<1>(key);
        get<1>(key) = dict_word;
        word.clear();

      } else {
        word += c;
      }
    } // c
  } // i

  cout << "dict: " << dict.size() << endl;
  cout << "map : " << map.size() << endl;

  /*
  for (const auto& x : map) {
    cout << "(" << x.first[0] << ',' << x.first[1] << ") =";
    for (const auto& w : x.second)
      cout <<' '<< w;
    cout << endl;
  }
  */

  for (std::string str; std::cin >> str; ) {
    unsigned pos;
    try {
      pos = boost::lexical_cast<unsigned>(str);
    } catch (std::exception& e) {
      pos = 0;
      for (const auto& x : map) {
        const auto& key = x.first;
        if (*get<0>(key)==str || *get<1>(key)==str)
          cout << get<0>(key) << ' ' << get<1>(key) << ' ' << pos << endl;
        ++pos;
      }
      continue;
    }
    auto& seed = select_rand(map,pos);
    key = seed.first;
    dict_word = select_rand(seed.second);

    cout << key[0] <<' '<< key[1] <<' '<< dict_word;
    for (unsigned i=0; i<35; ++i) {
      get<0>(key) = get<1>(key);
      get<1>(key) = dict_word;
      const auto it = map.find(key);
      if (it==map.end()) break;
      dict_word = select_rand(it->second);
      cout <<' '<< dict_word;
    }
    cout << endl;
  }
}
