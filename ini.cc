#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Section {
private:
  string title_;
  unordered_map<string, string> content_;

public:
  Section(){};
  const string &title = title_;

  void Create(string name) { title_ = name; }
  void SetContent(string key, string value) { content_[key] = value; }
  int Count() { return content_.size(); }
};

class INIFile {
private:
  string db_name_, db_path_;
  fstream fd_;
  vector<Section> sections_;

  static string *ProcessLine(string line, char delimiter = '=') {
    static string array[2];
    string p1, p2;
    size_t index;

    index = line.find(delimiter);
    if (index == string::npos) {
      return array;
    }
    array[0] = line.substr(0, index);
    array[1] = line.substr(index+1);
    return array;
  }

  string ExtractSectionName(string line) {
    size_t l_idx, r_idx;

    l_idx = line.find("[");
    r_idx = line.rfind("]");

    if (l_idx != string::npos && r_idx != string::npos) {
      return line.substr(l_idx + 1, r_idx - 1);
    }
    return "";
  }

public:
  INIFile(string name, string path = "db/") {
    db_name_ = name;
    db_path_ = path;
  }

  ~INIFile() {
    if (fd_) {
      fd_.close();
    }
  }

  string GetDBName() { return db_name_; }
  void SetDBName(string name) { db_name_ = name; }
  string GetDBPath() { return db_path_; }
  int GetSectionCount() { return sections_.size(); }
  void AddSection(Section section) { sections_.push_back(section); }

  int Open() {
    string file_path = db_path_ + db_name_ + ".ini";
    string line;
    int line_count = 0;

    fd_.open(file_path);
    if (!fd_) {
      return -1;
    }

    Section *current_section;
    while (getline(fd_, line)) {
      if (line[0] == '[') {
        string title = ExtractSectionName(line);
        if (!title.empty()) {
          Section section;
          section.Create(title);
          current_section = &section;
        }
      } else {
        if (!current_section->title.empty()) {
          string *line_splits;
          line_splits = INIFile::ProcessLine(line);
          if (!line_splits->empty()) {
            current_section->SetContent(line_splits[0], line_splits[1]);
            AddSection(*current_section);
          }
        }
      }
      line_count += 1;
    }
    return line_count;
  }

  Section *GetSection(string name) {
    for (size_t i = 0; i < sections_.size(); i++) {
      if (sections_[i].title == name) {
        return &sections_[i];
      }
    }
    return nullptr;
  }
};

/**************
 * TEST CASES *
 **************/
TEST_CASE("INIFile class test") {
  INIFile ini_file("XDF_Op_TigerTrap", "db/");

  SUBCASE("open an ini file") {
    int count = ini_file.Open();
    CHECK(count == 314);
  }
  SUBCASE("get total number of sections") {
    ini_file.Open();
    CHECK(ini_file.GetSectionCount() == 9);
  }
  SUBCASE("return session section from db") {
    ini_file.Open();
    Section *session = ini_file.GetSection("session");
    CHECK(session != nullptr);
  }
}
