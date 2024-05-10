@ECHO OFF

for /d %%a in (*) do (
  cd /d "%%a"
  # this assumes that you have added irfanview to your windows PATH
  i_view64.exe .\*.png /bpp=4 /convert=.\*.bmp
  cd ..
)

ECHO. "Finished Converting Images To 4bpp Bitmaps"