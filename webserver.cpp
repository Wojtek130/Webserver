#include "webserver.hpp"

// Wojciech Sniady, 322993

std::map<int, std::string> response_status = {
    {200, "OK"},
    {301, "Moved Permanently"},
    {403, "Forbidden"},
    {404, "Not found"},
    {501, "Not implemented"}
};

std::map<std::string, std::string> content_types = {
    {".txt", "text/plain;charset=utf-8"},
    {".html", "text/html;charset=utf-8"},
    {".css", " text/css"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".pdf", "application/pdf"},
    {"other", "application/octet-stream"},
};

bool port_validation(int port) {
    return (port >= 0 && port <= 65535);
}

bool directory_validation(std::string path) {
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

void handle_errors(int client_socket) {
    std::string response = generate_response(501);
    if (send(client_socket, response.c_str(), response.size(), 0) < 0) {
        std::cerr << "Sending data failed" << std::endl;
    }
    std::cout<<"forbidden 501\n";

}

std::string trim_leading_whitespace(std::string str) {
    size_t non_space_pos = str.find_first_not_of(" \t");
    return (non_space_pos != std::string::npos) ? str.substr(non_space_pos) : "";
}

std::string get_file_extension(std::string file_path) {
    std::filesystem::path path(file_path);
    return path.extension().string();
}

std::string get_domain_name(std::string request) {
    std::regex pattern(R"(Host: ([^:]+):)");
    std::smatch matches;
    if (std::regex_search(request, matches, pattern)) {
        return matches.str(1); 
    }
    return "virbian";
}

std::string generate_response(int status_code, std::string file_path) {
    std::string file_content = status_code == 200 ? get_file_content(file_path) : generate_html_error_description(status_code);
    size_t file_size = file_content.size();
    std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + response_status[status_code]+ "\r\n";
    std::string content_type = status_code == 200 ? content_types[get_file_extension(file_path)] : "text/html;charset=utf-8";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(file_size) + "\r\n";
    response += "\r\n";
    response += file_content;
    return response;
}

std::string generate_response_200(std::string file_path) {
    int status_code = 200;
    std::string file_content = get_file_content(file_path);
    size_t file_size = file_content.size();
    std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + response_status[status_code]+ "\r\n";
    std::string content_type = content_types[get_file_extension(file_path)];
    content_type = content_type.empty() ? content_types["other"] : content_type;
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(file_size) + "\r\n";
    response += "\r\n";
    response += file_content;
    return response;
}

std::string generate_html_error_description(int status_code) {
    std::string error_text = std::to_string(status_code) + " " + response_status[status_code];
    std::string html = "<!DOCTYPE html>";
    html += "<head>";
    html += "    <title>" + error_text + "</title>";
    html += "</head>";
    html += "<body>";
    html += "    <h1>" + error_text+ "</h1>";
    html += "</body>";
    html += "</html>";
    return html;

}


std::string generate_response_301(std::string new_loc) {
    int status_code = 301;
    std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + response_status[status_code]+ "\r\n";
    response += "Location: " + new_loc + "\r\n";
    response += "\r\n";
    return response;
}

std::string generate_response_501() {
    int status_code = 501;
    std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + response_status[status_code]+ "\r\n";
    response += "\r\n";
    return response;
}

std::string get_file_content(const std::string& file_path) {
    std::ifstream file(file_path);
    std::stringstream file_buffer;
    file_buffer << file.rdbuf();
    return file_buffer.str();
}

std::pair<std::string, bool> get_first_line_and_connection(std::string request) {
    std::string request_first_line;
    bool connection_close = false;
    std::istringstream iss(request);
    std::string curr_line;
    std::string conn_close = "Connection: close";
    int line_ind = 0;


    while (std::getline(iss, curr_line)) {
        if (curr_line.empty()) {
            continue;
        }
        if (line_ind == 0)
        {
            request_first_line = curr_line;
        }
        if (curr_line.compare(0, conn_close.length(), conn_close) == 0)
        {
            connection_close = true;
        }
        line_ind++;
    }
    return std::make_pair(request_first_line, connection_close);
}

std::vector<std::string> get_first_line_vector(std::string request_first_line) {
    std::istringstream ss(request_first_line);
    std::string part;
    std::vector<std::string> request_first_line_v;
    while(getline(ss, part, ' ')) {
        request_first_line_v.push_back(part);
    }
    if (request_first_line_v.size() != 3)
    {
        request_first_line_v.clear();
    }
    return request_first_line_v;
}

std::string convert_path(const std::string& path) {
    auto path_fs = std::filesystem::absolute(path);
    std::filesystem::path converted_path = path_fs.lexically_normal();
    return converted_path.string();
}

bool process_request(int client_socket, std::string request, std::string path_initial) {

    auto [request_first_line, connection_close] = get_first_line_and_connection(trim_leading_whitespace(request));
    std::string domain_name = get_domain_name(request);
    std::vector<std::string> request_first_line_v = get_first_line_vector(request_first_line);
    if (request_first_line_v.empty() || request_first_line_v[0] != "GET") {
        handle_errors(client_socket);
        return connection_close;
    }
    std::string endpoint = path_initial + "/" + domain_name;
    bool redirect = false;
    if (request_first_line_v[1] == "/")
    {
        redirect = true;
        endpoint += "/index.html";
    }
    else
    {
        endpoint += request_first_line_v[1];
    }
    std::string path = endpoint;
    std::string path_prefix = path_initial;
    std::filesystem::path path_obj(path);
    path = convert_path(path);
    path_prefix = convert_path(path_prefix);
    if (path_prefix.back() != '/') {
        path_prefix += "/";
    }
    path_prefix += domain_name;
    bool path_is_prefix = path.compare(0, path_prefix.length(), path_prefix) == 0 || path == path_prefix;
    bool path_exists = std::filesystem::is_regular_file(path);
    std::string response = "";
    if (!path_is_prefix)
    {
        std::cout<<"forbidden 403\n";
        response = generate_response(403);

    }
    else if (redirect)
    {
        std::cout<<"redirect 301\n";
        response = generate_response_301(request_first_line_v[1] + "index.html");
    }
    else if (path_exists)
    {
        std::cout<<"ok 200\n";
        response = generate_response(200, path);
    }
    else 
    {
        std::cout<<"not found 404\n";
        response = generate_response(404);
    }
    if (send(client_socket, response.c_str(), response.size(), 0) < 0) {
        handle_errors(client_socket);
    }
    std::cout<<"resource: "<<request_first_line_v[1]<<"\n";
    return connection_close;
}
