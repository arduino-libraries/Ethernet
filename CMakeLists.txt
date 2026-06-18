# register esp-idf component
if (ESP_PLATFORM)
    idf_component_register(
            SRCS
            "src/Dhcp.cpp"
            "src/Dns.cpp"
            "src/Ethernet.cpp"
            "src/EthernetClient.cpp"
            "src/EthernetServer.cpp"
            "src/EthernetUdp.cpp"
            "src/socket.cpp"
            "src/utility/w5100.cpp"
            INCLUDE_DIRS "src" "src/utility"
            REQUIRES arduino)
endif ()
