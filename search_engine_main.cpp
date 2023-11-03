#include <iostream>
#include <functional>
#include <string.h>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm> 
#include <regex>
#include <list>
#include <vector>


// function to parse an HTML file and extract links to local files
std::list<std::string> extractLinksFromHTML(const std::string& fileContent) {
    std::list<std::string> links;
    // regular expression to match href attributes in anchor tags
    std::regex linkRegex("<a\\s+[^>]*href\\s*=\\s*['\"]([^'\"]+)['\"][^>]*>");
    std::smatch match;

    // search for links in the HTML content
    std::string::const_iterator start = fileContent.cbegin();
    while (std::regex_search(start, fileContent.cend(), match, linkRegex)) {
        if (match.size() > 1) {
            links.push_back(match[1].str());
        }
        start = match.suffix().first;
    }

    return links;
}

//void function that takes in string reference, vector of string, and a string containing the URL, and the output
//designed to get all the url in the folders, adjusting the vector of string output;
void webCrawl(std::string& fileLink, std::vector<std::string> quiry, std::string URL, std::vector<std::string>& output) {
    //std::cout << "original: " << URL << std::endl;
    //get the links from the file
    std::list<std::string> links = extractLinksFromHTML(fileLink);
    std::string currOutput;
    output.push_back(URL);

    for(std::list<std::string>::iterator it = links.begin(); it!=links.end();it++) {

        std::string tempURL = (*it);
        //std::cout << tempURL << std::endl;
        std::string tempParent = URL.substr(0, URL.find_last_of('/'));
        while (tempURL.find("../") != std::string::npos) {
            // std::cout << "tempURL1: " << tempURL << std::endl;
            tempURL = tempURL.substr(tempURL.find("../") + 3);
            // std::cout << "tempURL2: " << tempURL << std::endl;
            // std::cout << "tempParent1: " << tempParent << std::endl;
            tempParent = tempParent.substr(0, tempParent.find_last_of('/'));
            // std::cout << "tempParent2: " << tempParent << std::endl;
        }

        std::string nextURL;
        if (URL != tempParent) {
            nextURL = tempParent + '/' + tempURL;
        } else {
            nextURL = URL.substr(0, URL.find_last_of('/')) + '/' + tempURL;
        }


        if(std::find(output.begin(),output.end(), nextURL) != output.end()) {
            continue;
        }

        std::string test = (nextURL).substr(URL.find_last_of('/')+1);
        std::ifstream nextFile(nextURL);
        std::string fileContent((std::istreambuf_iterator<char>(nextFile)), std::istreambuf_iterator<char>());
        webCrawl(fileContent,quiry,nextURL,output);
    }
}

std::vector<std::string> eraseBadURL(std::vector<std::string> URL, std::vector<std::string> query) {
    std::vector<std::string> tempURL = URL;
    bool isPhrase = false;
    std::string combinedQ;
    if(query.size() == 1) {
        isPhrase = true;
    }
    if(!isPhrase) {
        for(int i = 0; i < query.size(); i++) {
            combinedQ += query[i] + " ";
        }
    }
    for(int i = 0; i < tempURL.size();i++) {
        for(int j =0; j < query.size(); j++) {
            std::ifstream currFile(tempURL[i]);
            std::string fileContent((std::istreambuf_iterator<char>(currFile)), std::istreambuf_iterator<char>());
            if(fileContent.find(query[j]) == std::string::npos && isPhrase) {
                tempURL.erase(tempURL.begin()+i);
                i--;
                break;
            }
            else if(!isPhrase && fileContent.find(query[j]) == std::string::npos) {
                tempURL.erase(tempURL.begin()+i);
                i--;
                break;
            }
        }
    }

    return tempURL;
}

std::vector<std::string> queryOutput(std::vector<std::string> URL_list, std::vector<std::string> quiry) {
    std::vector<std::string> output;
    std::cout << "SIZE: " << URL_list.size() << std::endl;
    for(int i = 0; i < URL_list.size(); i++) {
        bool isPhrase = false;
        std::string currOutput;
        std::ifstream nextFile(URL_list[i]);
        std::string fileContent((std::istreambuf_iterator<char>(nextFile)), std::istreambuf_iterator<char>());
        std::string bodyFile = fileContent.substr(fileContent.find("<body>"));
        std::string currQuiry;
        int tempPos = 0;
        if(quiry.size() == 1) isPhrase = true;
        if(!isPhrase) {
            for(int i = 0;i < quiry.size();i++){
                currQuiry += quiry[i]+ " ";
            }
            currQuiry.erase(currQuiry.end()-1);
        }
        currOutput += "Title: " +fileContent.substr(fileContent.find("<title>")+7, 
        fileContent.find("</title>") - fileContent.find("<title>")-7) + "\n";

        currOutput += "URL: " + URL_list[i] + "\n";
        currOutput += "Description: " + fileContent.substr(fileContent.find("content=")+9, 
        fileContent.find("\">")-fileContent.find("content=")-9) + "\n";

        std::string descript = fileContent.substr(fileContent.find("content=")+9, 
        fileContent.find("\">")-fileContent.find("content=")-9);

        currOutput+="Snippet: ";
        std::string snippet;
        if(isPhrase && bodyFile.rfind(".", bodyFile.find(quiry[0])) != std::string::npos) {
            // std::cout << bodyFile << std::endl;
            std::cout << quiry[0] << std::endl;
            size_t pos = bodyFile.find(quiry[0]);
            std::cout <<"POS: " << pos <<" FILE: " << URL_list[i] << std::endl;
            //std::cout << bodyFile.substr(bodyFile.find(quiry[0]), bodyFile.find(quiry[0]) + 100) << std::endl;
            // if(descript.find(quiry[0]) != std::string::npos) {
            //     size_t nextPos = bodyFile.find(quiry[0],pos+1);
            //     size_t periodPos = bodyFile.rfind(".",nextPos);
            //     tempPos = periodPos;
            //     snippet += bodyFile.substr(periodPos+1,120);
            // }
            // else {
            size_t periodPos = bodyFile.rfind(".",pos);
            tempPos = periodPos;
            snippet += bodyFile.substr(periodPos+1, 120);
            //}
        }
        else if(!isPhrase&& bodyFile.rfind(".", bodyFile.find(currQuiry)) != std::string::npos && fileContent.find(currQuiry) !=std::string::npos) {
            size_t pos = bodyFile.find(currQuiry);
            // if(descript.find(currQuiry) != std::string::npos) {
            //     size_t nextPos = bodyFile.find(currQuiry,pos+1);
            //     size_t periodPos = bodyFile.rfind(".",nextPos);
            //     tempPos = periodPos;
            //     snippet += bodyFile.substr(periodPos+1,120);
            // }
            //else {
                size_t periodPos = bodyFile.rfind(".", bodyFile.find(currQuiry));
                tempPos = periodPos;
                snippet += bodyFile.substr(periodPos+1, +120);
            //}
        }
        else {
            size_t pos = bodyFile.find(quiry[0]);
            // if(descript.find(quiry[0]) != std::string::npos) {
            //     size_t nextPos = bodyFile.find(quiry[0],pos+1);
            //     size_t periodPos = bodyFile.rfind(".",nextPos);
            //     //std::cout << fileContent.find(quiry[0]) << std::endl;
            //     tempPos = periodPos;

            //     snippet += bodyFile.substr(periodPos+1,120);
            // }
            //else {
                size_t periodPos = bodyFile.rfind(".",pos);
                tempPos = periodPos;
                snippet += bodyFile.substr(periodPos+1, 120);
            //}
        }
        int emptySpace = 0;
        for(int j = 0; j < snippet.length(); j++) {
            if(!isspace(snippet[j])) {
                break;
            }
            else if(isspace(snippet[j])) {
                emptySpace++;
                snippet.erase(snippet.begin()+j);
                j--;
            }
        }
        // if(i == URL_list.size()) {
        //     snippet += bodyFile.substr(tempPos+121, emptySpace);
        // }
        // else {
            snippet += bodyFile.substr(tempPos+121, emptySpace) +"\n";

//        }
        currOutput += snippet;
        output.push_back(currOutput);
    }

    return output;
}
//SCORE

std::vector<double> keywordScore(std::vector<std::string> URL, std::vector<std::string> query, std::vector<std::string> allURL) {
    std::vector<double> keyDensity;
    double averageKeyboardScore = 0;
    double testScore = 0;
    double testScore3 = 0;
    double allCharacters = 0;
    double queryAppeared = 0;
    double testAppear = 0;
    double testAppear3 = 0;
    bool isPhrase = false;
    std::string combinedQ;
    if(query.size() == 1) {
        isPhrase = true;
    }
    if(!isPhrase) {

    }

    for(int i =0 ; i < allURL.size();i++) {
        std::ifstream nextFile(allURL[i]);
        std::string fileContent((std::istreambuf_iterator<char>(nextFile)), std::istreambuf_iterator<char>());
        allCharacters += fileContent.length();

    }
    for(int i = 0; i< URL.size();i++) {
        std::ifstream nextFile(URL[i]);
        std::string fileContent((std::istreambuf_iterator<char>(nextFile)), std::istreambuf_iterator<char>());
        // for(int j = 0; j < query.size(); j++) {
        //     if(isPhrase && fileContent.find(query[j]) != std::string::npos) {
        //         size_t pos = fileContent.find(query[j]);
        //         while(pos != std::string::npos) {
        //             queryAppeared++; 
        //             pos = fileContent.find(query[j], pos+1);
        //         }
        //     }
        //     else if(!isPhrase && fileContent.find(query[j]) != std::string::npos) {
        //         size_t pos = fileContent.find(query[j]);
        //         while(pos != std::string::npos) {
        //             queryAppeared++;
        //             pos = fileContent.find(query[j], pos + 1);
        //         }
        //     }
        // }

        if(isPhrase && fileContent.find(query[0]) != std::string::npos) {
            size_t pos = fileContent.find(query[0]);
            while(pos != std::string::npos) {
                queryAppeared++;
                pos = fileContent.find(query[0], pos+1);
            }
        }
        else if(!isPhrase && fileContent.find(query[0]) != std::string::npos) {
            size_t pos = fileContent.find(query[0]);
            while(pos != std::string::npos) {
                queryAppeared++;
                pos = fileContent.find(query[0], pos + 1);
            }
        }
        if(query.size() > 1) {
            if(isPhrase && fileContent.find(query[1]) != std::string::npos) {
                size_t pos = fileContent.find(query[1]);
                while(pos != std::string::npos) {
                    testAppear++;
                    pos = fileContent.find(query[1], pos+1);
                }
            }
            else if(!isPhrase && fileContent.find(query[1]) != std::string::npos) {
                size_t pos = fileContent.find(query[1]);
                while(pos != std::string::npos) {
                    testAppear++;
                    pos = fileContent.find(query[1], pos + 1);
                }
            }
        }
        if(query.size() > 2) {
            if(isPhrase && fileContent.find(query[2]) != std::string::npos) {
                size_t pos = fileContent.find(query[2]);
                while(pos != std::string::npos) {
                    testAppear3++;
                    pos = fileContent.find(query[2], pos+1);
                }
            }
            else if(!isPhrase && fileContent.find(query[2]) != std::string::npos) {
                size_t pos = fileContent.find(query[2]);
                while(pos != std::string::npos) {
                    testAppear3++;
                    pos = fileContent.find(query[2], pos + 1);
                }
            }
        }

    }
    
    averageKeyboardScore = (queryAppeared) / (allCharacters);

    if(query.size() > 1){
        testScore = (testAppear/allCharacters);
    }
    if(query.size() > 2) {
        testScore3 = testAppear3/allCharacters;
    }

    for(int i = 0; i< URL.size();i++) {
        double currQueryCount = 0;
        double currQueryCount2 = 0;
        double currQueryCount3 = 0;

        double currCharacters = 0;
        std::ifstream nextFile(URL[i]);
        std::string fileContent((std::istreambuf_iterator<char>(nextFile)), std::istreambuf_iterator<char>());
        currCharacters= fileContent.length();
        double currKeyDensity2 = 0;
        double currKeyDensity3 = 0;
        // for(int j = 0; j < query.size(); j++) {
        //     if(isPhrase && fileContent.find(query[j]) != std::string::npos) {
        //         size_t pos = fileContent.find(query[j]);
        //         while(pos != std::string::npos) {
        //             currQueryCount++;
        //             pos = fileContent.find(query[j], pos+1);
        //         }
        //     }
        //     else if(!isPhrase && fileContent.find(query[j]) != std::string::npos) {
        //         size_t pos = fileContent.find(query[j]);

        //         while(pos != std::string::npos) {
        //             // if(fileContent.substr(pos+query[j].length(), 1) == " ") {
        //             //     currQueryCount++;
        //             // }
        //             currQueryCount++;
        //             pos = fileContent.find(query[j], pos + 1);
        //             //currQueryCount++;
        //         }
        //     }
        // }

        if(isPhrase && fileContent.find(query[0]) != std::string::npos) {
            size_t pos = fileContent.find(query[0]);
            while(pos != std::string::npos) {
                currQueryCount++;
                pos = fileContent.find(query[0], pos+1);
            }
        }
        else if(!isPhrase && fileContent.find(query[0]) != std::string::npos) {
            size_t pos = fileContent.find(query[0]);

            while(pos != std::string::npos) {
                currQueryCount++;
                pos = fileContent.find(query[0], pos + 1);
            }
        }

        if(query.size() > 1) {
            if(isPhrase && fileContent.find(query[1]) != std::string::npos) {
                size_t pos = fileContent.find(query[1]);
                while(pos != std::string::npos) {
                    currQueryCount2++;
                    pos = fileContent.find(query[1], pos+1);
                }
            }
            else if(!isPhrase && fileContent.find(query[1]) != std::string::npos) {
                size_t pos = fileContent.find(query[1]);

                while(pos != std::string::npos) {
                    currQueryCount2++;
                    pos = fileContent.find(query[1], pos + 1);
                }
            }
            currKeyDensity2= currQueryCount2/ (currCharacters* testScore);

        }

        if(query.size() > 2) {
            if(isPhrase && fileContent.find(query[2]) != std::string::npos) {
                size_t pos = fileContent.find(query[2]);
                while(pos != std::string::npos) {
                    currQueryCount3++;
                    pos = fileContent.find(query[2], pos+1);
                }
            }
            else if(!isPhrase && fileContent.find(query[2]) != std::string::npos) {
                size_t pos = fileContent.find(query[2]);

                while(pos != std::string::npos) {
                    currQueryCount3++;
                    pos = fileContent.find(query[2], pos + 1);
                }
            }
            currKeyDensity3 = currQueryCount3/ (currCharacters* testScore3);
        }
        std::cout << "count: " << currQueryCount << std::endl;
        double currKeyDensity = 0 ;
        currKeyDensity = currQueryCount / (currCharacters * averageKeyboardScore);

        currKeyDensity += currKeyDensity2;
        currKeyDensity += currKeyDensity3;
        std::cout << URL[i] << ": " <<currKeyDensity  << std::endl;
        keyDensity.push_back(currKeyDensity);
        
    }

    return keyDensity;
}

std::vector<double> backLinkScore(std::vector<std::string> allURL, std::vector<std::string> currURL) {
    std::vector<double> backLink;

    for(int i = 0; i < currURL.size();i++) {
        currURL[i] = currURL[i].substr(currURL[i].find_last_of('/') + 1);
    }

    for(int i = 0; i < currURL.size();i++) {
        double var = 0;
        for(int j = 0; j < allURL.size(); j++) {
            std::ifstream nextFile(allURL[j]);
            std::string fileContent((std::istreambuf_iterator<char>(nextFile)), std::istreambuf_iterator<char>());

            std::list<std::string> links = extractLinksFromHTML(fileContent);
            for(std::list<std::string>::iterator it = links.begin(); it != links.end(); it++) {
                *it = (*it).substr((*it).find_last_of('/') + 1);
            }

            if(std::find(links.begin(),links.end(), currURL[i]) != links.end()) {
                var += 1.0/((double)links.size()+1);
            }
        }
        //std::cout <<"backlink: " <<  var << std::endl;
        std::cout << currURL[i] << ": " << var << std::endl;
        backLink.push_back(var);
    }
    return backLink;
}

bool comp(std::pair<std::string, double>& a, std::pair<std::string, double>& b) {
    return a.second > b.second;
}

std::map<double, std::string> makeMap(std::vector<double> backLink, std::vector<double> keywordScore, std::vector<std::string> queryOutput) {
    std::vector<double> pageRank;
    std::map<double, std::string> output;
    for(int i = 0; i < backLink.size(); i ++) {
        pageRank.push_back((backLink[i]*.5)+ +(keywordScore[i]*.5));
    }

    for(int i = 0; i < queryOutput.size(); i++) {
        output[pageRank[i]] = queryOutput[i];
    }



    return output;
}


int main(int argc, char* argv[]) {
    // if(argc > 6) {
    //     std::cout << "Please limit to 3 words for search inquiry\n";
    //     return 1;
    // }
    std::ifstream fileStream(argv[1]);
    std::ofstream out_str(argv[2]);
    std::vector<std::string> quiry;
    std::vector<std::string> phraseQ;
    bool isPhrase = false;
    if(!fileStream.good()) {
        std::cerr << "Can't open " << argv[1] << std::endl;
        return 1;
    }
    if(!out_str.good()) {
        std::cerr << "Can't open " << argv[2] << std::endl;
        return 1;
    }

    for(int i = 3; i < argc; i++) {
        quiry.push_back(argv[i]);
        std::cout << quiry[i] <<std::endl;
    } 

    quiry[0].erase(remove( quiry[0].begin(), quiry[0].end(), '\"' ),quiry[0].end());

    if(fileStream.is_open()) {
        std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        std::vector<std::string> wtf;
        webCrawl(fileContent, quiry, argv[1], wtf);
        std::vector<std::string> test = eraseBadURL(wtf, quiry);
        std::vector<std::string> queryOut = queryOutput(test, quiry);


        std::vector<double> backScore = backLinkScore(wtf, test);
        std::vector<double> keyDen;
        if(quiry.size() == 1){
            std::stringstream ss(quiry[0]);
            std::string word;
            while(ss >> word) {
                phraseQ.push_back(word);
            }
            keyDen = keywordScore(test,phraseQ,wtf);

        }   
        else {
            keyDen = keywordScore(test,quiry,wtf);
        }

        std::map<double, std::string> outputMap = makeMap(backScore, keyDen, queryOut);
        if(test.size() > 0) {
            out_str << "Matching documents: " << "\n\n";
            for(std::map<double, std::string>::reverse_iterator mapIt = outputMap.rbegin(); mapIt != outputMap.rend();
            ++mapIt) {
                out_str << mapIt->second << std::endl;
            }
        }
        else {
            out_str << "Your search - ";
            for(int i = 0; i < quiry.size(); i++) {
                out_str << quiry[i] << " ";
            }
            out_str << "- did not match any documents.";
        }
    }
}


//find the start of sentence
//size_t periodPos = data.rfind(".", queryPos);