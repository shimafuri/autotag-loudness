cpplint --filter=-whitespace/line_length,-legal/copyright --quiet main.cpp && g++ main.cpp -I/usr/include/taglib -L/usr/lib/x86_64-linux-gnu -ltag -lstdc++fs
