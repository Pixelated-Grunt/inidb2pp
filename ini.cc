#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "doctest.h"

using namespace std;

class Section {
 private:
  string title_;
  map<string, string> content_;

 public:
  Section(){};

  void Create(string name) { title_ = name; }
  void SetContent(string key, string value) { content_[key] = value; }
  void ClearContent() { content_.clear(); }
  int Count() { return content_.size(); }
  string GetTitle() { return title_; }
  void SetTitle(string new_title) { title_ = new_title; }
  string GetValue(string key) {
    if (auto v = content_.find(key); v != content_.end()) {
      return v->second.erase(v->second.find_last_of('\r'));
    } else {
      return "";
    }
  }
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
    array[1] = line.substr(index + 1);
    return array;
  }

  static string ExtractSectionName(string line) {
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
  vector<string> ListSections() {
    vector<string> titles = {};
    for (auto s : sections_) {
      titles.push_back(s.GetTitle());
    }
    return titles;
  }

  void AddSection(Section section) { sections_.push_back(section); }

  int Open() {
    string file_path = db_path_ + db_name_ + ".ini";
    string line;
    int line_count = 0;

    fd_.open(file_path);
    if (!fd_) {
      return -1;
    }

    Section current_section;
    while (getline(fd_, line)) {
      if (line[0] == '[') {
        string title = ExtractSectionName(line);
        if (!title.empty()) {
          // first entry
          if (current_section.GetTitle().empty()) {
            current_section.Create(title);
          } else {
            if (current_section.GetTitle() != title) {
              vector<string> target = ListSections();
              if (!(find(target.begin(), target.end(), title) !=
                    target.end())) {
                AddSection(current_section);
                current_section.ClearContent();
                current_section.Create(title);
              } else {
                current_section.SetTitle("");
              }
            }
          }
        }
      } else {
        if (!current_section.GetTitle().empty()) {
          string *line_splits;
          line_splits = INIFile::ProcessLine(line);
          if (!line_splits->empty()) {
            current_section.SetContent(line_splits[0], line_splits[1]);
          }
        }
      }
      line_count += 1;
    }
    if ((!current_section.GetTitle().empty()) &&
        (current_section.Count() > 0)) {
      AddSection(current_section);
    }
    return line_count;
  }

  Section *GetSection(string name) {
    for (size_t i = 0; i < sections_.size(); i++) {
      if (sections_[i].GetTitle() == name) {
        return &sections_[i];
      }
    }
    // TODO: change to exception
    return nullptr;
  }
};

/**************
 * TEST CASES *
 **************/
TEST_CASE("INIFile class function tests") {
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

    {
      Section *s = ini_file.GetSection("meta");
      CHECK(s != nullptr);
      CHECK(s->GetTitle() == "meta");
      CHECK(s->Count() == 9);
    }

    {
      Section *s = ini_file.GetSection("session");
      CHECK(s != nullptr);
      CHECK(s->GetTitle() == "session");
      CHECK(s->Count() == 22);
    }

    {
      Section *s = ini_file.GetSection("markers");
      CHECK(s != nullptr);
      CHECK(s->GetTitle() == "markers");
      CHECK(s->Count() == 106);
    }
  }
  SUBCASE("get value from key") {
    ini_file.Open();

    {
      Section *s = ini_file.GetSection("session");
      CHECK(s->GetValue("session.start.vehicles") == "\"21\"");
      CHECK(s->GetValue("session.start") == "\"[2024,5,26,17,24,23,399]\"");
    }

    {
      Section *s = ini_file.GetSection("markers");
      CHECK(s->GetValue("_USER_DEFINED #0/107/0") ==
            "\"\"~_USER_DEFINED "
            "#0/107/"
            "0~[5590.6641,8799.0645,0.0000]~hd_dot~ICON~[]~[1,1]~0~Solid~"
            "ColorGreen~1~41 LOC\"\"");
    }
  }
}

TEST_CASE("INIFile error tests") {
  INIFile ini_file("XDF_Op_Errors", "db/");
  ini_file.Open();

  SUBCASE("duplicate sections") { CHECK(ini_file.GetSectionCount() == 2); }
}
