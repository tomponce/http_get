/* WCCLIENT */
/* Sends HTTP GET requests to a remote server*/
/* Creates a cache of that webpage if response is OK */
/* Stores the the URL and the name of its cache file in list.txt */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void parsedate(char*, char*);
 
int main ()
{
    int sockfd, n, portno;
    int len = sizeof(struct sockaddr);
    char recvline[40960], url[1024], getreq[1024];
    char url_buff[1024], filename[15];
    struct sockaddr_in servaddr;
    FILE *fp;

    //attempt to open list.txt
    fp = fopen("list.txt", "r+");
    if(!fp) //if it fails, create it
    {
        fp = fopen("list.txt", "w+");
    }

    while(1) 
    {
        //create socket
        //bzero(url, sizeof(url));
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family = AF_INET;

        //ask user for URL and port no
        printf("URL: ");
        scanf("%s", url);
        if(!strcmp(url, "quit") || !strcmp(url, "Quit"))
            break;
        printf("Port: ");
        scanf("%d", &portno);
            
        //check if URL exists in cache already
        fseek(fp, 0, SEEK_SET); //reset file pointer to beginning of file
        while(!feof(fp))
        {
            fscanf(fp, "%s %s",url_buff,filename);
            //if url has been found
            if(!strcmp(url, url_buff))
            {
                printf("Response: 200\nAction: Page found in cache, filename - %s\n", filename);
                break;
            }
            //i++;
        }
        //don't call get request if url is already in the cache
        if(!strcmp(url, url_buff))
        {
            continue;
        }

        //retrieve IP address of URL
        struct hostent *addr = gethostbyname(url);
        //convert the address into an in_addr structs and assigns it  
        servaddr.sin_addr = *(struct in_addr *) (addr->h_addr);
        servaddr.sin_port = htons(portno); // Server port number

        //set get request
        bzero(getreq, sizeof(getreq));
        strcpy(getreq, "GET / HTTP/1.1\r\nHost: ");
        strcat(getreq, url);
        strcat(getreq, "\r\nConnection: Close\r\n\r\n\0");

        //connect to the server
        if (connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
        {
            printf("connect error\n");
            exit(EXIT_FAILURE);
        }

        send(sockfd, getreq, strlen(getreq), 0); // send get request to the server

        sleep(1);

        bzero(recvline, sizeof(recvline));

        n = read(sockfd, recvline, sizeof(recvline)); // read response from server

        //get response code and check if it's 200 before caching
        char response[] = {recvline[9], recvline[10], recvline[11], '\0'}; //get response code
        //if not 200 do not cache
        if(strcmp(response, "200"))
        {
            printf("Response: %s\n",response);
            printf("Action: Not cached\n");
            close(sockfd);
            continue;
        }

        //get date for name of cache file
        parsedate(recvline, filename);

        //create new file to store cache
        FILE *cachefp;
        if ((cachefp = fopen(filename, "w")) == 0)
            printf("Failed to create cache file\n");
    
        // write received text to cache file
        fprintf(cachefp,"%s",recvline);
        while (n > 0) //keep reading from server
        {
            bzero(recvline, sizeof(recvline));
            n = read(sockfd, recvline, sizeof(recvline));
            fprintf(cachefp,"%s",recvline); 
        }
        
        //count number of lines in list.txt
        int lines = 0;
        char dates[5][15]; //save file names for comparison
        fseek(fp, 0, SEEK_SET);
        while(!feof(fp))
        {
            fscanf(fp, "%*s %s\n", dates[lines]);
            lines++;
        }
        //if 5 or more lines find oldest cached file and delete it
        if(lines > 4)
        {
            int oldln = 0;
            for(int i = 1; i < 5; i++)
            {
                if(strcmp(dates[oldln], dates[i]) > 0)
                {
                    oldln = i;
                }
            }
            remove(dates[oldln]); //delete file by that name
            //delete from list.txt
            fseek(fp, 0, SEEK_SET);
            FILE *tmpfp;
            tmpfp = fopen("temp.txt", "w"); //create a temp file for new list.xt
            char tmpurl[1024];
            for(int i = 0; i < 5; i++)
            {
                fscanf(fp, "%s %*s\n", tmpurl);
                //skip line we want to remove
                if (i == oldln)
                    continue;
                fprintf(tmpfp, "%s %s\n", tmpurl, dates[i]);
            }
            fclose(fp);
            fclose(tmpfp);
            remove("list.txt"); //delete old list.txt
            rename("temp.txt", "list.txt"); //rename temp file to list.txt
            fp = fopen("list.txt", "r+"); //reopen
            fseek(fp, 0, SEEK_END);
        }

        //add new url and filename to list.txt
        fprintf(fp, "%s %s\n", url, filename); //add url and filename to list.txt
        printf("Response: %s\n",response);
        printf("Action: Page cached as filename - %s\n",filename);
        
        fclose(cachefp);
        close(sockfd);
    }
    fclose(fp);
    return 0;
}

void parsedate(char *http, char *filename)
{
    char *currln = strstr(http, "Date: ");
    //0          11 14  18   23 26 29
    //Date: Thu, 24 Feb 2022 12:06:42 GMT
    //filename is YYYYMMDDhhmmss format
    filename[0] = currln[18];
    filename[1] = currln[19];
    filename[2] = currln[20];
    filename[3] = currln[21];
    char *month = currln + 14;
    //convert month to MM format
    if(strncmp(month, "Jan", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '1';
    }else if(strncmp(month, "Feb", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '2';
    }else if(strncmp(month, "Mar", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '3';
    }else if(strncmp(month, "Apr", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '4';
    }else if(strncmp(month, "May", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '5';
    }else if(strncmp(month, "Jun", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '6';
    }else if(strncmp(month, "Jul", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '7';
    }else if(strncmp(month, "Aug", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '8';
    }else if(strncmp(month, "Sep", 3) == 0)
    {
        filename[4] = '0';
        filename[5] = '9';
    }else if(strncmp(month, "Oct", 3) == 0)
    {
        filename[4] = '1';
        filename[5] = '0';
    }else if(strncmp(month, "Nov", 3) == 0)
    {
        filename[4] = '1';
        filename[5] = '1';
    }else if(strncmp(month, "Dec", 3) == 0)
    {
        filename[4] = '1';
        filename[5] = '2';
    }
    filename[6] = currln[11];
    filename[7] = currln[12];
    filename[8] = currln[23];
    filename[9] = currln[24];
    filename[10] = currln[26];
    filename[11] = currln[27];
    filename[12] = currln[29];
    filename[13] = currln[30];
    filename[14] = '\0';
}