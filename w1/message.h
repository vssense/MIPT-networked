#pragma once

static constexpr const char* kClientPort = "2022"; 
static constexpr const char* kServerPort = "2023";
static constexpr size_t kBufSize = 1024;

enum MessageType {
    kInitConnection,
    kSendMessage,
    kKeepAlive
};

