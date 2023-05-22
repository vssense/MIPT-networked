g++ -o lobby -I ../3rdParty/enet/include/ lobby.cpp ../3rdParty/enet/libenet.a
g++ -o client -I ../3rdParty/enet/include/ client.cpp ../3rdParty/enet/libenet.a
g++ -o server -I ../3rdParty/enet/include/ server.cpp ../3rdParty/enet/libenet.a
