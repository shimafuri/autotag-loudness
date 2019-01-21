#include <stdio.h>
#include <stdlib.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/mp4tag.h>

#include <experimental/filesystem>
#include <regex>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>

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

  // 1引数を強制
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
      if (!f.isNull() && f.tag()) { // 音楽ファイルかつタグ情報があった場合
        ++numAudioFiles;

        std::cout << p << std::endl; // ファイルパスを出力

        std::string lufs_integrated = "AAAAA"; // 初期値
        std::string lufs_max = "BBBBB";
        std::string lufs_min = "CCCCC";
        std::string lufs_range = "DDDDD";

        // std::cout << "Calculating loudness..." << std::endl;
        { // FFmpeg を実行
          std::string command = "ffmpeg -i \"" + std::string(s) + "\" -filter_complex ebur128 -f null - 2> test.txt";

          // std::cout << command << std::endl;
          std::system(command.c_str());
        }
        { // FFmpeg の出力の末尾 9 行を切り出し
          std::string command = "tail -n 9 test.txt > test_tail.txt";

          // std::cout << command << std::endl;
          std::system(command.c_str());
        }
        { // FFmpeg の出力の末尾 9 行を出力
          int count(0);
          std::ifstream file("test_tail.txt");
          for (std::string line ; std::getline(file, line) ; ) {
            switch (count) {
              case 1: // Integrated loudness [LUFS]
                // std::cout << line << std::endl;
                { // 値を正規表現で切り出して変数に代入
                  std::smatch results;
                  if (std::regex_match(line, results, std::regex("^\\s+I:\\s+(-?\\d+(\\.\\d+))\\s+LUFS$"))) {
                    lufs_integrated = results[1].str();
                    // std::cout << lufs_integrated << std::endl;
                  }
                }
                break;
              case 5: // Loudness range [LU]
                // std::cout << line << std::endl;
                { // 値を正規表現で切り出して変数に代入
                  std::smatch results;
                  if (std::regex_match(line, results, std::regex("^\\s+LRA:\\s+(-?\\d+(\\.\\d+))\\s+LU$"))) {
                    lufs_range = results[1].str();
                    // std::cout << lufs_range << std::endl;
                  }
                }
                break;
              case 7: // Min loudness [LUFS]
                // std::cout << line << std::endl;
                { // 値を正規表現で切り出して変数に代入
                  std::smatch results;
                  if (std::regex_match(line, results, std::regex("^\\s+LRA low:\\s+(-?\\d+(\\.\\d+))\\s+LUFS$"))) {
                    lufs_min = results[1].str();
                    // std::cout << lufs_min << std::endl;
                  }
                }
                break;
              case 8: // Max loudness [LUFS]
                // std::cout << line << std::endl;
                { // 値を正規表現で切り出して変数に代入
                  std::smatch results;
                  if (std::regex_match(line, results, std::regex("^\\s+LRA high:\\s+(-?\\d+(\\.\\d+))\\s+LUFS$"))) {
                    lufs_max = results[1].str();
                    // std::cout << lufs_max << std::endl;
                  }
                }
                break;
              default:
                break;
            }
            ++count;
          }
        }
        std::cout << "Loudness: " << lufs_integrated << " LUFS (Range: " << lufs_range << " LU) (Min: " << lufs_min << " LUFS) (Max: " << lufs_max << " LUFS)" << std::endl;

        // タグを読み込んで書き込み
        if (p.extension() == ".m4a" || p.extension() == ".mp4") {
          TagLib::MP4::Tag* mp4tag = dynamic_cast<TagLib::MP4::Tag*>(f.file()->tag());
          TagLib::MP4::ItemMap items = mp4tag->itemMap(); // タグを読み込み
          // { // タグをダンプ
          //   std::cout << "-- TAG (items) --" << std::endl;
          //   for (TagLib::MP4::ItemMap::ConstIterator i = items.begin(); i != items.end(); ++i) {
          //     TagLib::StringList list = i->second.toStringList();
          //     for (TagLib::StringList::ConstIterator j = list.begin(); j != list.end(); ++j) {
          //       std::cout << i->first << " - " << '"' << (*j).toCString(true) << '"' << std::endl;
          //     }
          //   }
          // }
          { // タグを書き込み
            mp4tag->setItem("----:com.apple.iTunes:lufs_integrated", TagLib::MP4::Item(TagLib::StringList(lufs_integrated)));
            mp4tag->setItem("----:com.apple.iTunes:lufs_max", TagLib::MP4::Item(TagLib::StringList(lufs_max)));
            mp4tag->setItem("----:com.apple.iTunes:lufs_min", TagLib::MP4::Item(TagLib::StringList(lufs_min)));
            mp4tag->setItem("----:com.apple.iTunes:lufs_range", TagLib::MP4::Item(TagLib::StringList(lufs_range)));
          }
          // std::cout << "All tags were successfully set." << std::endl;
          mp4tag->save(); // ファイルを保存
        } else {
          TagLib::PropertyMap props = f.file()->properties(); // タグを読み込み
          // { // タグをダンプ
          //   std::cout << "-- TAG (properties) --" << std::endl;
          //   for (TagLib::PropertyMap::ConstIterator i = props.begin(); i != props.end(); ++i) {
          //     for (TagLib::StringList::ConstIterator j = i->second.begin(); j != i->second.end(); ++j) {
          //       std::cout << i->first << " - " << '"' << (*j).toCString(true) << '"' << std::endl;
          //     }
          //   }
          // }
          { // タグを準備
            props.replace(TagLib::String("lufs_integrated"), TagLib::String(lufs_integrated));
            props.replace(TagLib::String("lufs_max"), TagLib::String(lufs_max));
            props.replace(TagLib::String("lufs_min"), TagLib::String(lufs_min));
            props.replace(TagLib::String("lufs_range"), TagLib::String(lufs_range));
          }
          { // タグを書き込み
            const TagLib::PropertyMap rejectedTags = f.file()->setProperties(props);
            // if (rejectedTags.size() == 0) {
            //   std::cout << "All properties were successfully set." << std::endl;
            // } else {
            //   std::cout << "Some properties were rejected." << std::endl;
            // }
          }
          f.file()->save(); // ファイルを保存
        }
      } else {
        ++numNonAudioFiles;
      }
      std::cout << std::endl;
    }
  }
  // std::cout << "\33[2K\r";
  std::cout << "Done." << std::endl;
  std::cout << "Non-audio files: " << numNonAudioFiles << std::endl;
  std::cout << "Audio files: " << numAudioFiles << std::endl;
}
