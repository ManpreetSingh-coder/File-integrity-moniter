#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <map>
#include <filesystem>
#include <openssl/evp.h>

using namespace std;
namespace fs = std::filesystem;


string hashFile(const string& filepath){
    ifstream file(filepath, ios::binary);
    if(!file) {
        cerr << "could not open file: " << filepath << endl;
        return "";
     }


    vector<char> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());


unsigned char hash[EVP_MAX_MD_SIZE];
unsigned int hashLen = 0;

EVP_Digest(buffer.data(), buffer.size(), hash, &hashLen, EVP_sha256(), NULL);

ostringstream oss;
for(unsigned int i = 0; i < hashLen; i++) {
   oss << hex << setw(2) << setfill('0') << (int)hash[i];
}

   return oss.str();
}

map<string, string> scanDirectory(const string& dirPath){
  map<string, string> results;

  for (fs::recursive_directory_iterator it(dirPath), end; it != end; ++it) {
     const fs::directory_entry& entry = *it;


     if (entry.is_regular_file()) {
        string path = entry.path().string();

        if (path == "./baseline.txt" || path == dirPath + "/baseline.txt") {
         continue;
        }

     string hash = hashFile(path);
     results[path] = hash;
  }
}


  return results;
}


void saveBaseline(const map<string, string>& results, const string& outputFile) {
  ofstream out(outputFile);
  if (!out) {
      cerr <<  "could not open baseline file for writing: " << outputFile << endl;
      return;
}

for(map<string, string>::const_iterator it = results.begin(); it != results.end(); ++it) {
    out << it->first << "," << it->second << endl;
}

  out.close();
}



map<string, string> loadBaseline(const string& inputFile) {
   map<string, string> results;
   ifstream in(inputFile);

   if(!in) {
     cerr << "could not open baseline file for reading: " << inputFile << endl;
     return results;
}

string line;
while (getline(in, line)) {
   size_t commaPos = line.find_last_of(',');
   if (commaPos == string::npos) continue;

   string path = line.substr(0, commaPos);
   string hash = line.substr(commaPos + 1);
   results[path] = hash;
}


return results;

}


void compareBaseline(const map<string, string>& oldBaseline, const map<string, string>& newScan) {

  for (map<string, string>::const_iterator it = oldBaseline.begin(); it != oldBaseline.end(); ++it) {
     string path = it->first;
     string oldHash = it->second;

     map<string, string>::const_iterator found = newScan.find(path);

     if (found == newScan.end()) {
        cout << "[DELETED] " << path <<endl;
     }
     else if (found->second != oldHash) {
       cout << "[MODIFIED] " << path << endl;
     }
}

for (map<string, string>::const_iterator it = newScan.begin(); it != newScan.end(); ++it) {
        string path = it->first;

   map<string, string>::const_iterator found = oldBaseline.find(path);

   if (found == oldBaseline.end()) { 
      cout << "[NEW]      " << path << endl;
    }
  }
}




int main(int argc, char* argv[]){
   if (argc < 3) {
      cerr << "Usage:" << endl;
      cerr << "  " << argv[0] << " --baseline <folder>" << endl;
      cerr << "  " << argv[0] << " --scan <folder>" << endl;
      return 1;
    }

   string mode  = argv[1];
   string folder = argv[2];


   if (mode == "--baseline") {
     map<string, string> results = scanDirectory(folder);
     saveBaseline(results, "baseline.txt");
     cout << "Baseline saved for: " << folder << endl;
   }


   else if (mode == "--scan") {
     map<string, string> loadedBaseline = loadBaseline("baseline.txt");
     map<string, string> currentScan = scanDirectory(folder);


     cout << "Comparing..." << endl;
     compareBaseline(loadedBaseline, currentScan);
     cout << "Done." << endl;
}

  else{
     cerr << "Unknown mode: " << mode << endl;
     return 1;
}

return 0;

}
// test comment
