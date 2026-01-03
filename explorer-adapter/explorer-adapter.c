#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winternl.h>

// Function to remove quotes from a string
void remove_quotes(char *str) {
    size_t len = strlen(str);
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

// Function to remove spaces from a string
void remove_spaces(char *str) {
    size_t len = strlen(str);
    if (len >= 2 && str[0] == ' ' && str[len - 1] == ' ') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

// Function to check if a string starts with a drive letter
int starts_with_drive_letter(const char *str) {
    if (strlen(str) >= 2) {
        char c = toupper(str[0]);
        return (c >= 'A' && c <= 'Z' && str[1] == ':');
    }
    return 0;
}

// Function to check if a single argument contains multiple, like "/select,C:\Path" style 
void parse_single_arg(char *arg, int *is_select, char **target_path) {
    *is_select = 0;
    char working_arg[1024];
    strcpy(working_arg, arg);
    remove_quotes(working_arg);
    
    // Check for /select, or /root
    char *comma = strchr(working_arg, ',');
    if (comma) {
        if (strncmp(working_arg, "/select", 7) == 0) {
            *is_select = 1;
            *comma = '\0'; // Temporarily split the string
            *target_path = comma + 1;
            // Skip optional space after comma
            while (**target_path == ' ') (*target_path)++;
        } else if (strncmp(working_arg, "/root", 5) == 0) {
            *comma = '\0';
            *target_path = comma + 1;
            while (**target_path == ' ') (*target_path)++;
        }
        else {
            *target_path = strdup(arg);
        }
    }
    else {
        *target_path = strdup(arg);
    }
}

// Function to execute a command
void execute_command(const char *command) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcess(NULL, (LPSTR)command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("Failed to execute command: %s\n", command);
    }
}

void execute_fallback_command(int argc, char *argv[]) {
    char cmd[1024] = {0};
    
    const char *env_cmd = getenv("EXPLORER_REDIRECT_FALLBACK");
    if (env_cmd != NULL && strlen(env_cmd) > 0) {
        strncpy(cmd, env_cmd, sizeof(cmd) - 1);
    } else {
        strncpy(cmd, "C:\\Windows\\explorer.exe", sizeof(cmd) - 1);
    }
    
    // Append each argument to the command
    for (int i = 1; i < argc; i++) {
        strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, argv[i], sizeof(cmd) - strlen(cmd) - 1);
    }
    
    execute_command(cmd);
}

int main(int argc, char *argv[]) {
    int is_select = 0;
    char *target_path = NULL;
    
    // Parse arguments
    if(argc == 2) {
        parse_single_arg(argv[1], &is_select, &target_path);
    }
    else if (argc > 2 && (strcmp(argv[1], "/root") == 0 || strcmp(argv[1], "/select") == 0)) {
        is_select = (strcmp(argv[1], "/select") == 0);
        target_path = strdup(argv[2]);
    }
    else {
        printf("Executing fallback: No arguments passed\n");
        execute_fallback_command(argc, argv);
        return 0;
    }
    
    // Clean up the target path
    char cleaned_path[1024];
    strcpy(cleaned_path, target_path);
    free(target_path);
    remove_quotes(cleaned_path);
    // Remove file:/// prefix if present
    if (strncmp(cleaned_path, "file:///", 8) == 0) {
        memmove(cleaned_path, cleaned_path + 8, strlen(cleaned_path) - 7);
    }
    // Check if path starts with a drive letter
    if (!starts_with_drive_letter(cleaned_path)) {
        printf("Executing fallback: Argument %s is not a Windows path\n", cleaned_path);
        execute_fallback_command(argc, argv);
        return 0;
    }
    
    // Convert to Unix path
    WCHAR wide_path[1024];
    MultiByteToWideChar(CP_ACP, 0, cleaned_path, -1, wide_path, sizeof(wide_path) / sizeof(WCHAR));
    char *unix_path = wine_get_unix_file_name(wide_path);
    if (!unix_path || strlen(unix_path) == 0) {
        printf("Executing fallback: Failed to convert path \"%s\" to Unix path\n", cleaned_path);
        execute_fallback_command(argc, argv);
        return 0;
    }
    
    // Determine redirect handler path
    char *wineprefix = getenv("WINEPREFIX");
    if (!wineprefix || strlen(wineprefix) == 0) {
        wineprefix = "~/.wine"; // There might be a better way to determine the default?
    }
    // Remove trailing slash if present
    size_t prefix_len = strlen(wineprefix);
    if (prefix_len > 0 && wineprefix[prefix_len - 1] == '/') {
        wineprefix[prefix_len - 1] = '\0';
    }
    char redirect_handler[1024];
    snprintf(redirect_handler, sizeof(redirect_handler), "%s/explorer-redirect/redirect-path", wineprefix);
    
    // Prepare the command
    char command[2048];
    if (is_select) {
        printf("Selecting \"%s\" converted from \"%s\"\n", unix_path, cleaned_path);
        snprintf(command, sizeof(command),
                 "start /unix /bin/bash -c \"\"%s\" --select '%s'\"",
                 redirect_handler, unix_path);
    } else {
        printf("Opening \"%s\" converted from \"%s\"\n", unix_path, cleaned_path);
        snprintf(command, sizeof(command),
                 "start /unix /bin/bash -c \"\"%s\" '%s'\"",
                 redirect_handler, unix_path);
    }
    
    // Execute the command
    execute_command(command);   
    return 0;
}

