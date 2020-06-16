# Turn orange.png into Uint8Array that can be easily read by C++

from PIL import Image
import numpy as np
import pathlib

img = Image.open(pathlib.Path(__file__).parent.absolute() / "orange.png")

np.array(img).astype(np.uint8).flatten().tofile(
    pathlib.Path(__file__).parent.parent / "assets/orange.bin"
)
