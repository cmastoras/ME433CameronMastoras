import lyricsgenius as genius
api = genius.Genius('tJEazB1jCEgPTCgXkkqHgvgBJG-vTEdFJfCg0l0SpSSbwBO-TKqRiibvhs6xydKz')
artist = api.search_artist('Andy Shauf', max_songs=3)
print(artist.songs)