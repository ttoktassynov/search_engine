#include "search_server.h"
#include "iterator_range.h"
#include "profile.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <future>
#include <mutex>
#include <thread>
#include <chrono>

const size_t MAX_DOC = 50'000;
const size_t MAX_WORD = 1'000;
const size_t MAX_ALIKE = 10'000;
const size_t MAX_CHAR = 100;
const size_t MAX_QUERY = 500'000;
const size_t MAX_WORD_PER_QUERY = 10;

vector<string> SplitIntoWords(const string& line) {
  istringstream words_input(line);
  return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream& document_input) {
  UpdateDocumentBase(document_input);
}

// Update
void SearchServer::UpdateDocumentBaseThread(istream& document_input, InvertedIndex& index)
{
    InvertedIndex new_index;
    for (string current_document; getline(document_input, current_document); ) {//N
      new_index.Add(move(current_document));//K
    }
    lock_guard<mutex> g(index_mutex);
    swap(index, new_index);
}

void SearchServer::UpdateDocumentBase(istream& document_input) 
{ //NK = 50'000*1'000=50'000'000
   index_mutex.lock();
   size_t n = index.GetSize();
   index_mutex.unlock();
   if (n == 0)
   {
     UpdateDocumentBaseThread(document_input, index);
   }
   else
   {
     futures.push_back(
       async(&SearchServer::UpdateDocumentBaseThread, this, ref(document_input), ref(index))
     );
   }
}
// Search

void SearchServer::AddQueriesStream(istream& query_input, ostream& search_results_output)
{
  futures.push_back(
      async(&SearchServer::AddQueriesStreamThread, this, ref(query_input), ref(index), ref(search_results_output))
    );
  this_thread::sleep_for(chrono::milliseconds(5));
}
void SearchServer::AddQueriesStreamThread( 
  istream& query_input, InvertedIndex& index, ostream& search_results_output)
 {
  index_mutex.lock();
  size_t n = index.GetSize();
  index_mutex.unlock();
  vector<pair<size_t, size_t> > docid_count(n);

  for (string current_query; getline(query_input, current_query);) { //C=500'000
    const auto words = SplitIntoWords(current_query);//Q = 10
    docid_count.assign(n, {0, 0});
    // Calculation
    for (const auto& word : words) {//10* N
      {
        index_mutex.lock();
        auto entry = index.Lookup(word);
        index_mutex.unlock();
        for (const auto stat : entry) {//N
          docid_count[stat.first].first = stat.first;
          docid_count[stat.first].second += stat.second;
        }
      }
    }

    size_t mid = min<size_t>(5, docid_count.size());
    partial_sort( //NLogN = 16*50'000 = 750'000
      begin(docid_count),
      begin(docid_count) + mid,
      end(docid_count),
      [](pair<size_t, size_t> lhs, pair<size_t, size_t> rhs) {
        int64_t lhs_docid = lhs.first;
        auto lhs_hit_count = lhs.second;
        int64_t rhs_docid = rhs.first;
        auto rhs_hit_count = rhs.second;
        return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
      }
    );
    
    lock_guard<mutex> g(output_mutex);
    search_results_output << current_query << ':';
    for (auto [docid, hitcount] : Head(docid_count, 5)) { //N
      if (hitcount > 0)
      {
         search_results_output << " {"
        << "docid: " << docid << ", "
        << "hitcount: " << hitcount << '}';
      }
    }
    search_results_output << '\n';
  }
}
// Add
void InvertedIndex::Add(const string& document) {
  //docs.push_back(document);

  const size_t docid = cur_size++;
  unordered_map<string, size_t> temp;
  for (const auto& word : SplitIntoWords(document)) { // aba aba aba ddc // 1000
    temp[word]++;
  }//K
  for (const auto& [word, count]: temp)//500
  {
    index[word].push_back(make_pair(docid, count));
  }
}

vector<pair<size_t, size_t> > InvertedIndex::Lookup(const string& word) const {
  if (auto it = index.find(word); it != index.end()) {
    return it->second;
  } else {
    return {};
  }
}
