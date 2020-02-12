package main

import (
	"fmt"
	"image"
	_ "image/gif"
	_ "image/jpeg"
	_ "image/png"
	"os"
	"strconv"
)

func main() {
	fileName, divNum := getArgs()
	file, err := os.Open(fileName)
	if err != nil {
		fmt.Println("Failed to open", fileName)
		os.Exit(1)
	}
	defer file.Close()
	im, _, err := image.Decode(file)
	if err != nil {
		fmt.Println("Failed to decode", fileName, "(is it an image file?)")
		os.Exit(1)
	}
	procedure(im, divNum)
}

func getArgs() (string, int) {
	argsCnt := len(os.Args)
	if argsCnt < 2 {
		fmt.Println("usage:", os.Args[0], "image_file [division_number]")
		os.Exit(1)
	}
	divNum := 1
	if argsCnt > 2 {
		divNum, _ = strconv.Atoi(os.Args[2])
	}
	return os.Args[1], divNum
}

func procedure(im image.Image, divNum int) {
	x, y, w, h := 0, 0, im.Bounds().Dx()/divNum, im.Bounds().Dy()
	fmt.Println("#define IMG_WIDTH ", w)
	fmt.Println("#define IMG_HEIGHT", h)
	fmt.Printf("PROGMEM static const uint8_t bitmap[%d][%d] = {\n", divNum, ((w+7)/8)*h)
	for i := 0; i < divNum; i++ {
		convertImage(im, x, y, w, h)
		x += w
	}
	fmt.Println("};")
}

func convertImage(im image.Image, x int, y int, w int, h int) {
	fmt.Print("    { ")
	for dy := 0; dy < h; dy++ {
		value := 0
		for dx := 0; dx < w; dx++ {
			r, g, b, a := im.At(x+dx, y+dy).RGBA()
			if a >= 128 && r*3+g*6+b >= 1280 {
				value += 1 << uint(7-dx%8)
			}
			if dx%8 == 7 || dx == w-1 {
				fmt.Printf("0x%02X, ", value)
				value = 0
			}
		}
	}
	fmt.Println("},")
}
