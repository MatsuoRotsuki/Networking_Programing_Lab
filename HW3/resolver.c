#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>

#define MAX_URL_LENGTH 2048

/**
 * @brief check if host information is found
 * 
 * @param hostInfo 
 */
void checkHostInfo(struct hostent *hostInfo);

/**
 * @brief detect IP address to domain name
 * 
 * @param ip 
 */
void detectIP(struct in_addr ip);

/**
 * @brief detect domain name to IP address
 * 
 * @param domain 
 */
void detectDomain(char const *domain);

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t realsize = size * nmemb;
  char *data = (char *)userdata;
  strcat(data, ptr);
  return realsize;
}

void saveLinkToCSV(char *data) {
  regex_t regex;
  int reti;
  char msgbuf[100];
  regmatch_t pmatch[1];
  char url[MAX_URL_LENGTH];
  FILE *fp;

  // Compile the regular expression
  reti = regcomp(&regex, "https://[^\" ]+", REG_EXTENDED);
  if (reti) {
    fprintf(stderr, "Could not compile regex\n");
    exit(1);
  }

  // Open the output file
  fp = fopen("links.csv", "w");
  if (fp == NULL) {
    fprintf(stderr, "Could not open output file\n");
    exit(1);
  }

  // Search for matches in the data string
  char *ptr = data;
  while (1) {
    reti = regexec(&regex, ptr, 1, pmatch, 0);
    if (reti == REG_NOMATCH) {
      break;
    } else if (reti) {
      regerror(reti, &regex, msgbuf, sizeof(msgbuf));
      fprintf(stderr, "Regex match failed: %s\n", msgbuf);
      exit(1);
    }

    // Copy the matched URL to the output buffer
    int len = pmatch[0].rm_eo - pmatch[0].rm_so;
    strncpy(url, ptr + pmatch[0].rm_so, len);
    url[len] = '\0';

    // Remove unwanted characters after the URL
    for (int i = len - 1; i >= 0; i--) {
      if (url[i] == ')' || url[i] == '\n' || url[i] == '\r') {
        url[i] = '\0';
      } else {
        break;  // Stop at the first non-unwanted character
      }
    }

    // Write the URL to the output file
    fprintf(fp, "%s\n", url);

    // Move the pointer to the next character after the match
    ptr += pmatch[0].rm_eo;
  }

  // Clean up
  regfree(&regex);
  fclose(fp);
}


void crawlWebPage(char const *url) {
  CURL *curl;
  CURLcode res;
  char *data = malloc(sizeof(char) * 1000000);
  data[0] = '\0';

  curl = curl_easy_init();
  if (curl == NULL) {
    printf("Error initializing curl\n");
    exit(1);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    free(data);
    data = NULL;
  }
  curl_easy_cleanup(curl);

  saveLinkToCSV(data);
  free(data);
}

const char *addHttpsPrefix(char const *domain ) {
  const char *httpsPrefix = "https://";
  int urlSize = strlen(httpsPrefix) + strlen(domain) + 1;
  char *url = (char *)malloc(urlSize);

  strcpy(url, httpsPrefix);
  strcat(url, domain);

  return url;
  free(url);
}


int main(int argc, char const *argv[])
{
  if (argc != 2)
  {
    perror("Please enter domain or ip address\n");
    exit(0);
  }

  struct in_addr ip;

  // Check if input is an IP address
  if (inet_aton(argv[1], &ip) != 0)
  {
    detectIP(ip);
  }
  else
  {
    // Input is not an IP address, assume it's a domain name
    detectDomain(argv[1]);
    crawlWebPage(addHttpsPrefix(argv[1]));
  }

  return 0;
}


void checkHostInfo(struct hostent *hostInfo)
{
  if (hostInfo == NULL)
  {
    printf("not found information\n");
    exit(0);
  }
}

void detectIP(struct in_addr ip)
{
  char *IPbuffer;
  struct hostent *host_info;
  inet_aton(IPbuffer, &ip);

  // To retrieve host information
  host_info = gethostbyaddr(&ip, sizeof(ip), AF_INET);
  checkHostInfo(host_info);

  FILE *fp;
  fp = fopen("result.csv", "a+");

  if (fp == NULL) {
    perror("Error opening file.\n");
    exit(0);
  }

  fprintf(fp, "Official name: %s\n", host_info->h_name);
  crawlWebPage(addHttpsPrefix(host_info->h_name));

  char **alias_names = host_info->h_aliases;
  fprintf(fp, "Alias name:\n");

  for (int i = 0; alias_names[i] != NULL; i++) {
    fprintf(fp, "%s\n", alias_names[i]);
  }

  fclose(fp);
}

void detectDomain(char const *domain) {
  struct hostent *host_info;

  // Detect host information
  host_info = gethostbyname(domain);
  checkHostInfo(host_info);

  // Convert an Internet network to address into ASCII string
  char *ip_string = inet_ntoa(*((struct in_addr *)host_info->h_addr_list[0]));

  FILE *fp;
  fp = fopen("result.csv", "w+");

  if (fp == NULL) {
    perror("Error opening file.\n");
    exit(0);
  }

  fprintf(fp, "Official IP: %s\n", ip_string);
  fprintf(fp, "Alias IP:\n");

  // print alias ip
  int i = 1;
  while (host_info->h_addr_list[i] != NULL) {
    fprintf(fp, "%s\n", inet_ntoa(*(struct in_addr *)host_info->h_addr_list[i]));
    i++;
  }
}