# Search Engine

Search engine is C++ console application that enables users to add documents of words to database and perform queries.
The API of *SearchServer* is following:

```

class SearchServer {
public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);

  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);
};
```

## AddQueriesStream method

It accepts input stream of search queries *query_input*, and output stream of search results *search_results_output* to write to.

The search result is set of maximum 5 most relevant documents. The document **relevancy** is calculated as sum of hit counts of each query words in the document. Suppose, we need to search query *the best capital* in the database that has three documents:

- *"london is the capital of great britain"* 
- *"moscow is the capital of the russian federation"*
- *"paris is the capital of france"

Then relevancy metrics of those documents would be:

- *"london is the capital of great britain"* - 2 ("the" word appeared 1 time, "best" - 0, "capital" - 1)
- *"moscow is the capital of the russian federation"* - 3 ("the" word appeared 2 times, "best" — 0, "capital" — 1)
- *"paris is the capital of france" - 2 ("the" — 1, "best" — 0, "capital" — 1)

Thus, the *"moscow is the capital of the russian federation"* is the most relevant document to abovementioned search query.

## UpdateDocumentBase method

This methods replaces the content of the database with a new set of documents.
For example, the following code:

```
const string doc1 = "london is the capital of great britain";
const string doc2 = "moscow is the capital of the russian federation";
istringstream doc_input1(doc1 + '\n' + doc2);
SearchServer srv(doc_input1);

const string query = "the capital";
istringstream query_input1(query);
srv.AddQueriesStream(query_input1, cout);

istringstream doc_input2(doc2 + '\n' + doc1);
srv.UpdateDocumentBase(doc_input2);
istringstream query_input2(query);
srv.AddQueriesStream(query_input2, cout);
```

Should return:

```
the capital: {docid: 1, hitcount: 3} {docid: 0, hitcount: 2}
the capital: {docid: 0, hitcount: 3} {docid: 1, hitcount: 2}
```


To get started:

- Clone this project
- Add this build task if you are using VS code:

```
{
	"version": "2.0.0",
	"tasks": [
		{
			"type": "cppbuild",
			"label": "C/C++: clang++ build directory",
			"command": "/usr/bin/clang++",
			"args": [
				"-std=c++2a",
				"-stdlib=libc++",
				"-g",
				"${fileDirname}/*.cpp",
				"-o",
				"${fileDirname}/${fileBasenameNoExtension}"
			],
			"options": {
				"cwd": "${workspaceFolder}"
			},
			"problemMatcher": [
				"$gcc"
			],
			"group": {
				"kind": "build",
				"isDefault": true
			},
			"detail": "compiler: /usr/bin/clang++"
		}
	]
}
```
- Run "C/C++: clang++ build directory" run task to compile
- Run ./main from command line
- Create your own test using *TestFunctionality()* template
