from lyricsgenius import Genius

genius = Genius('tJEazB1jCEgPTCgXkkqHgvgBJG-vTEdFJfCg0l0SpSSbwBO-TKqRiibvhs6xydKz')
artist = genius.search_artist('Action Bronson',max_songs = 3)
artist.save_lyrics()