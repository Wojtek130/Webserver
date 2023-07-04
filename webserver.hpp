#pragma once

// Wojciech Sniady, 322993

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <numeric>
#include <regex>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <utility> 
#include <vector>
#include <fcntl.h>


const int MAX_LISTENERS_NO = 5;
const int MESSAGE_LENGTH = 2048;
const int DELAY = 1000000;


bool directory_validation(std::string path);
bool port_validation(int port);
bool process_request(int client_socket, std::string request, std::string path_initial);
std::vector<std::string> get_first_line_vector(std::string request_first_line);
std::pair<std::string, bool> get_first_line_and_connection(std::string request);
std::string get_file_content(const std::string& file_path);
std::string convert_path(const std::string& path);
std::string get_domain_name(std::string request);
std::string generate_response(int status_code, std::string file_path="");
std::string generate_response_200(std::string file_path);
std::string generate_response_301(std::string new_loc);
std::string generate_response_501();
std::string generate_html_error_description(int status_code);
void handle_errors(int client_socket);
std::string get_file_extension(std::string file_path);
std::string trim_leading_whitespace(std::string str);
