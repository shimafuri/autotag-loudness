cpplint --filter=-whitespace/line_length,-whitespace/comments,-legal/copyright,-build/c++11 --quiet main.cpp && g++ main.cpp -I/usr/include/taglib -L/usr/lib/x86_64-linux-gnu -ltag -lstdc++fs
