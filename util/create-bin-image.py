# Turn orange.png into Uint8Array that can be easily read by C++

from PIL import Image
import numpy as np
import pathlib
import sys

# img = Image.open(pathlib.Path(__file__).parent.absolute() / "orange.png")
img = Image.open(sys.argv[1])

img.putalpha(255)

np.array(img).astype(np.uint8).flatten().tofile(sys.argv[2])
