#pragma once

#include "iterator_range.h"
#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <mutex>
#include <future>
#include <shared_mutex>
using namespace std;

template <typename T>
class Synchronized {
public:
  explicit Synchronized(T initial = T()) :
    value_(move(initial)) {}

  struct Access {
    lock_guard<mutex> guard;
    T& ref_to_value;
  };
  Access GetAccess()
  {
    return {lock_guard(m_), value_};
  }
private:
  T value_;
  mutex m_;
};

class InvertedIndex {
public:
  void Add(const string& document);
  vector<pair<size_t, size_t> > Lookup(const string& word) const;
  const size_t GetSize() const{
    return cur_size;
  }
  /*const string& GetDocument(size_t id) const {
    return docs[id];
  }*/

private:
  unordered_map<string, vector<pair<size_t,size_t> > > index;
  size_t cur_size = 0;
};

class SearchServer {
public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void UpdateDocumentBaseThread(istream& document_input, InvertedIndex& index);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);
  void AddQueriesStreamThread(istream& query_input, InvertedIndex& index, ostream& search_results_output);

private:
  InvertedIndex index;
  mutex index_mutex;
  mutex output_mutex;
  vector<future<void> > futures;
  vector<future<string> > search_futures;
};
