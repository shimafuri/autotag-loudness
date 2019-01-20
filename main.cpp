#include <stdio.h>
#include <stdlib.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#include <experimental/filesystem>
#include <iostream>
#include <iomanip>

namespace fs = std::experimental::filesystem;

// from https://stackoverflow.com/questions/19580877/how-to-truncate-a-string-formating-c
std::string truncate(std::string str, size_t width, bool show_ellipsis = true) {
  if (str.length() > width) {
    if (show_ellipsis) {
      return str.substr(0, width) + "...";
    } else {
      return str.substr(0, width);
    }
  }
  return str;
}

/**
 * target-dir 内の音楽ファイルのラウドネス関連値を FFmpeg で計算し、タグに書き込みます。
 * 
 * Usage:
 *   autotag-loudness <target-dir>
 */
int main(int argc, char* argv[]) {
  int numNonAudioFiles(0);
  int numAudioFiles(0);

  if (argc != 2) {
    std::cout << "Usage: autotag-loudness <target-dir>" << std::endl;
    return EXIT_FAILURE;
  }

  for (const fs::directory_entry& entry : fs::recursive_directory_iterator(argv[1])) {
    // std::cout << "\33[2K\r" << truncate(entry.path(), 60) << std::flush;

    const fs::path p = entry.path();
    const char* s = p.c_str();
    // 全ての通常のファイルを処理
    if (fs::is_regular_file(p)) {
      TagLib::FileRef f(s);
      if (!f.isNull() && f.tag()) {
        // std::cout << " -> Audio file!" << std::endl;
        ++numAudioFiles;
        std::cout << p << std::endl;
      } else {
        // std::cout << " -> x" << std::endl;
        ++numNonAudioFiles;
      }
    }
  }
  // std::cout << "\33[2K\r";
  std::cout << "Done." << std::endl;
  std::cout << "Non-audio files: " << numNonAudioFiles << std::endl;
  std::cout << "Audio files: " << numAudioFiles << std::endl;
}
