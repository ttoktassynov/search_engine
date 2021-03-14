#include "search_server.h"
#include "iterator_range.h"
#include "profile.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>

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

void SearchServer::UpdateDocumentBase(istream& document_input) { //NK = 50'000*1'000=50'000'000
  InvertedIndex new_index;

  for (string current_document; getline(document_input, current_document); ) {//N
    new_index.Add(move(current_document));//K
  }

  index = move(new_index);
}

void SearchServer::AddQueriesStream(
  istream& query_input, ostream& search_results_output
) {
  vector<pair<size_t, size_t> > docid_count(MAX_DOC);
  for (string current_query; getline(query_input, current_query); ) { //C=500'000
    const auto words = SplitIntoWords(current_query);//Q = 10
    docid_count.assign(MAX_DOC, {0, 0});
    for (const auto& word : words) {//10* N
        for (const auto stat : index.Lookup(word)) {//N
          docid_count[stat.first].first = stat.first;
          docid_count[stat.first].second += stat.second;
        }
    }

    size_t mid = min<size_t>(5, docid_count.size());
    partial_sort( //N
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

    search_results_output << current_query << ':';
    for (auto [docid, hitcount] : Head(docid_count, 5)) { //5
      if (hitcount > 0)
      {
         search_results_output << " {"
        << "docid: " << docid << ", "
        << "hitcount: " << hitcount << '}';
      }
    }
    search_results_output << endl;
  }
}

void InvertedIndex::Add(const string& document) {
  docs.push_back(document);

  const size_t docid = docs.size() - 1;
  unordered_map<string, size_t> temp;
  for (const auto& word : SplitIntoWords(document)) { // aba aba aba ddc
    temp[word]++;
  }//K = 1000
  for (const auto& [word, count]: temp)
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
