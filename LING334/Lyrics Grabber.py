from lyricsgenius import Genius
import re
from bs4 import BeautifulSoup
import requests
import urllib
import urllib.parse
import json
import csv
import urllib.request
import pandas as pd
import numpy as np
genius = Genius('tJEazB1jCEgPTCgXkkqHgvgBJG-vTEdFJfCg0l0SpSSbwBO-TKqRiibvhs6xydKz')

client_access_token ='tJEazB1jCEgPTCgXkkqHgvgBJG-vTEdFJfCg0l0SpSSbwBO-TKqRiibvhs6xydKz'
#api = genius.Genius(client_access_token)
#artist = genius.search_artist('Action Bronson',max_songs = 1)
#artist.save_lyrics()
# white_rappers = ['Eminem','Beastie Boys','Yelawolf','Mac Miller','G-Easy','Macklemore','Machine Gun Kelly','Lil Dicky','Action Bronson', 'NF','Mike Shinoda','House of Pain','R.A. The Rugged Man'] 
# white_rappers2 = ['Rittz' ,'Brother Ali' ,'Everlast', 'Token','Kid Rock', 'Paul Wall','Post Malone','Weird Al Yankovic','Vinnie Paz','Hoodie Allen' ,'Asher Roth' ]

# black_rappers3 = ['Ice cube','Nas','Kendrick Lamar','2pac','The Notorious B.I.G.','J. Cole','Snoop Dogg','Lil Wayne','Jay-Z','Kayne West','Gucci Mane','50 Cent','Eazy-E','MF DOOM','Travis Scott']
# black_rappers2 = ['Kid Cudi','ASAP Rocky','Nicki Minaj','DMX','Tyler, The Creator','Childish Gambino','Logic','Nate Dogg','Joey Bada$$']

black_rappers = ['Nate Dogg','Snoop Dogg','Ice cube','Nas','Kendrick Lamar','2pac','J. Cole','Lil Wayne','Jay-Z','Kayne West','Gucci Mane','50 Cent','Eazy-E','MF DOOM','Travis Scott','Kid Cudi','ASAP Rocky','Nicki Minaj','DMX','Tyler, The Creator','Childish Gambino','Logic','Joey Bada$$']

artist_list = ['Ice cube','Nas','Kendrick Lamar','2pac','The Notorious B.I.G.']
num_songs = 10

songs = genius.search_songs('Baby Blue')
lyrics = []
def testfun():
    for song in songs['hits']:
        url = song['result']['url']
        song_lyrics = genius.lyrics(song_url=url)
        #id = song['result']['id']
        #song_lyrics = genius.lyrics(id)
        lyrics.append(song_lyrics)
        print("yessir")

def lyric_grabber(artist,numsongs):
    search_term = artist
    _URL_API = "https://api.genius.com/"
    _URL_SEARCH = "search?q="
    querystring = _URL_API + _URL_SEARCH + urllib.parse.quote(search_term)
    request = urllib.request.Request(querystring)
    request.add_header("Authorization", "Bearer " + client_access_token)
    request.add_header("User-Agent", "")
    response = urllib.request.urlopen(request, timeout=3)
    #json_obj = response.json()
    data = response.read()
    encoding = response.info().get_content_charset('utf-8')
    json_obj = json.loads(data.decode(encoding))
    URLS = []
    for i in range(0,numsongs):
        URLS.append(json_obj['response']['hits'][i]['result']['url'])
    print(URLS)
    
    lyric_list = []
    for url in URLS:
        #URL = 'https://genius.com/Andy-shauf-the-magician-lyrics'
        URL = url
        page = requests.get(URL)
        html = BeautifulSoup(page.text, "html.parser") # Extract the page's HTML as a string
        #print(html)
        # Scrape the song lyrics from the HTML
        lyricsnow = html.find("div", class_="Lyrics__Container").get_text()
        lyric_list.append(lyricsnow)
    return lyric_list
TOTAL_SONG_LIST = []

class Song:
    def __init__(self,year,race,lyrics):
        self.year = year
        self.race = race
        self.lyrics = lyrics
    
    def line_ize(self):
        #print(self.lyrics)
        self.lyrics = self.lyrics.replace("("," ")
        self.lyrics = self.lyrics.replace(")"," ")
        self.lines = list(filter(None,re.split("[\r\n]|\[.*\]",self.lyrics)))
        #print(self.lines)
    def tokenize(self):
        print("HELLOW")


def lyric_grabber2(artistname,numsongs,race):
    try:
        artist = genius.search_artist(artistname, max_songs=numsongs)
    
        artist.save_lyrics()
        parsedname = artistname.replace(" ","")
        parsedname = artistname.replace("$","")
        parsedfilename = "Lyrics_"+str(parsedname)+".json"
        #Artistread=pd.read_json("Lyrics_ActionBronson.json")
        parsedfilename = parsedfilename.replace("-","")
        
        parsedfilename = parsedfilename.replace(" ","")
        with open(parsedfilename, 'r',encoding="utf-8") as myfile:
            data=myfile.read()
        
        #json_obj = json.loads("Lyrics_ActionBronson.json")
        json_obj = json.loads(data)
        lyric_list = []
        for i in range(0,numsongs):
            date = re.split("-",json_obj['songs'][i]['release_date'])
            lYrics = json_obj['songs'][i]['lyrics']
            #print(lYrics)
            newsong = Song(date[0],race,lYrics)
            newsong.line_ize()
            TOTAL_SONG_LIST.append(newsong)
        print("added all "+str(numsongs)+" "+str(artistname)+" songs to the master list")
    except:
        print("Aww shucks")
  
def artist_runner(artistlist,numberofsongs):
    print("what race is "+str(artistlist[0]))
    race = input()
    for i in range(len(artistlist)):
        lyric_grabber2(artistlist[i], numberofsongs, race)




def TOTAL_SONG_COMPILER(artistlist):
    print("FUCK")


biglist = []

def CSV_Saver(songlist,title):
    
    for song in songlist:
        for line in song.lines:
            var = line.replace('\u200b','')
            var = line.replace('\u0435','')
            
            listhing = [song.race,song.year,var]
            biglist.append(listhing)
    with open('100songs2.json', 'w',encoding="utf-8") as f:
        json.dump(biglist, f)


    # np.savetxt(title, 
    #        biglist,
    #        delimiter =", ", 
    #        fmt ='% s')

    
# artist_runner(black_rappers,2)
# CSV_Saver(TOTAL_SONG_LIST,"B1.csv")
# TOTAL_SONG_LIST = []
# artist_runner(black_rappers2,2)
# CSV_Saver(TOTAL_SONG_LIST,"B2.csv")
# TOTAL_SONG_LIST = []
# artist_runner(white_rappers,2)
# CSV_Saver(TOTAL_SONG_LIST,"W1.csv")
# TOTAL_SONG_LIST = []
# artist_runner(white_rappers2,2)
# CSV_Saver(TOTAL_SONG_LIST,"W2.csv")
# TOTAL_SONG_LIST = []

artist_runner(black_rappers,2)
CSV_Saver(TOTAL_SONG_LIST,"Test.csv")

#artist_runner(black_rappers,2)
#artist_runner(artist_list,2)
#print(TOTAL_SONG_LIST[1].lyrics)
#CSV_Saver(TOTAL_SONG_LIST,"Test.csv")