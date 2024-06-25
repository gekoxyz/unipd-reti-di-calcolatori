unsigned char envbuf[1000];
int pid;
int env_i, env_c;
char* env[100];
int new_stdin, new_stdout;
char* myargv[10];

void add_env(char* env_key, char* env_value) {
  sprintf(envbuf + env_c, "%s=%s", env_key, env_value);
  env[env_i++] = envbuf + env_c;
  env_c += (strlen(env_value) + strlen(env_key) + 2);
  env[env_i] = NULL;
}

if (!strncmp(url, "/cgi/", 5)) {  // CGI
  filename = url + 5;
  if (!strcmp(method, "GET")) {
    for (i = 0; filename[i] && (filename[i] != '?'); i++);
    if (filename[i] == '?') {
      filename[i] = 0;
      add_env("QUERY_STRING", filename + i + 1);
    }
    add_env("CONTENT_LENGTH", "0");
  }
  else if (!strcmp(method, "POST")) {
    char tmp[10];
    sprintf(tmp, "%d", length);
    add_env("CONTENT_LENGTH", tmp);
  }
  else {
    sprintf(response, "HTTP/1.1 501 Not Implemented\r\n\r\n");
    write(s2, response, strlen(response));
    close(s2);
    continue;
  }
  fin = fopen(filename, "rt");
  if (fin == NULL) {
    sprintf(response, "HTTP/1.1 404 Not Found\r\n\r\n");
    write(s2, response, strlen(response));
  }
  else {
    sprintf(response, "HTTP/1.1 200 OK\r\n\r\n");
    write(s2, response, strlen(response));
    fclose(fin);
    for (i = 0; env[i]; i++)
      printf("environment: %s\n", env[i]);
    sprintf(fullname, "/home/utente/%s", filename);
    myargv[0] = fullname;
    myargv[1] = NULL;
    printf("Executing %s\n", fullname);
    if (!(pid = fork())) {
      dup2(s2, 1);
      dup2(s2, 0);
      if (-1 == execve(fullname, myargv, env))
      {
        perror("execve");
        exit(1);
      }
    }
    waitpid(pid, NULL, 0);
    printf("Il processo figlio e' terminato...\n");
  }
}