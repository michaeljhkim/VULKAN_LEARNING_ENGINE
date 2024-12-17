//alright, once I do have the general file with all of the tiles, how do I then stream it?

/*
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <unordered_set>
#include "stb_image.h"

// Tile dimensions
const int TILE_WIDTH = 128;
const int TILE_HEIGHT = 128;
const int TILE_SIZE = TILE_WIDTH * TILE_HEIGHT * 4; // RGBA (4 bytes per pixel)

// Function to hash the entire texture data (for texture existence check)
size_t hashTextureData(const unsigned char* data, int width, int height) {
    size_t hash = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4; // RGBA
            for (int c = 0; c < 4; ++c) {
                hash = hash * 31 + data[idx + c];
            }
        }
    }
    return hash;
}

// Function to hash individual tile data
size_t hashTileData(const unsigned char* data, size_t size) {
    size_t hash = 0;
    for (size_t i = 0; i < size; ++i) {
        hash = hash * 31 + data[i];
    }
    return hash;
}

struct Tile {
    int offset;    // File offset of the tile data
    size_t hash;   // Hash of the tile data
};

// Function to save the packed texture tiles
void saveTextureTiles(const unsigned char* data, int width, int height, const std::string& filename) {
    int tilesX = width / TILE_WIDTH;
    int tilesY = height / TILE_HEIGHT;

    std::vector<Tile> tiles;
    std::vector<unsigned char> tileData;

    // Open the file for binary writing
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    // Write the header
    int totalTiles = tilesX * tilesY;
    outFile.write(reinterpret_cast<const char*>(&totalTiles), sizeof(totalTiles));
    outFile.write(reinterpret_cast<const char*>(&TILE_WIDTH), sizeof(TILE_WIDTH));
    outFile.write(reinterpret_cast<const char*>(&TILE_HEIGHT), sizeof(TILE_HEIGHT));

    // Calculate the texture hash
    size_t textureHash = hashTextureData(data, width, height);
    outFile.write(reinterpret_cast<const char*>(&textureHash), sizeof(textureHash));

    // Current position of the offset table in the file (right after the header)
    int offsetTablePosition = outFile.tellp();
    outFile.write(reinterpret_cast<const char*>(&offsetTablePosition), sizeof(offsetTablePosition));

    // Save tile data
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            Tile tile;
            tile.offset = outFile.tellp(); // Store current file position for this tile

            // Extract the tile pixels
            for (int y = 0; y < TILE_HEIGHT; ++y) {
                for (int x = 0; x < TILE_WIDTH; ++x) {
                    int srcX = tx * TILE_WIDTH + x;
                    int srcY = ty * TILE_HEIGHT + y;
                    int srcIdx = (srcY * width + srcX) * 4;

                    // Copy the RGBA data for this pixel into tileData buffer
                    for (int c = 0; c < 4; ++c) {
                        tileData.push_back(data[srcIdx + c]);
                    }
                }
            }

            // Compute the tile's hash
            tile.hash = hashTileData(tileData.data(), tileData.size());

            // Write tile data to the file
            outFile.write(reinterpret_cast<const char*>(tileData.data()), tileData.size());
            tileData.clear();

            // Store the tile metadata (in this case, we're just storing the offset and hash)
            tiles.push_back(tile);
        }
    }

    // Write the offset table (start at offsetTablePosition)
    outFile.seekp(offsetTablePosition);
    for (const Tile& tile : tiles) {
        outFile.write(reinterpret_cast<const char*>(&tile.offset), sizeof(tile.offset));
        outFile.write(reinterpret_cast<const char*>(&tile.hash), sizeof(tile.hash));
    }

    outFile.close();
    std::cout << "Texture tiles packed successfully!" << std::endl;
}

// Function to load the texture (using stb_image)
unsigned char* loadTexture(const std::string& path, int& width, int& height) {
    unsigned char* data = stbi_load(path.c_str(), &width, &height, nullptr, 4); // Load as RGBA
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return nullptr;
    }
    return data;
}

// Function to check if all tiles for a texture exist in the packed file
bool textureTilesExist(const std::string& filename, const unsigned char* textureData, int width, int height) {
    // Hash of the entire texture
    size_t textureHash = hashTextureData(textureData, width, height);

    // Open the packed texture file
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Failed to open file for reading." << std::endl;
        return false;
    }

    // Read the texture hash from the packed file
    size_t storedTextureHash;
    inFile.seekg(sizeof(int) * 3, std::ios::beg); // Skip header
    inFile.read(reinterpret_cast<char*>(&storedTextureHash), sizeof(storedTextureHash));

    // If the texture hash doesn't match, the texture doesn't exist
    if (storedTextureHash != textureHash) {
        inFile.close();
        return false;
    }

    // Read the tile offset table position
    int offsetTablePosition;
    inFile.read(reinterpret_cast<char*>(&offsetTablePosition), sizeof(offsetTablePosition));

    // Read the tile metadata (offsets and hashes)
    std::vector<size_t> tileHashes;
    inFile.seekg(offsetTablePosition, std::ios::beg);

    // Extract tile data and check hashes
    int totalTiles;
    inFile.read(reinterpret_cast<char*>(&totalTiles), sizeof(totalTiles));

    std::vector<size_t> textureTileHashes(totalTiles);
    for (int i = 0; i < totalTiles; ++i) {
        size_t tileHash;
        inFile.read(reinterpret_cast<char*>(&tileHash), sizeof(tileHash));
        textureTileHashes[i] = tileHash;
    }

    // Check if all tiles exist
    bool allTilesExist = true;
    int tilesX = width / TILE_WIDTH;
    int tilesY = height / TILE_HEIGHT;

    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            int tileIndex = ty * tilesX + tx;

            // Compute the hash for the current tile
            std::vector<unsigned char> tileData;
            for (int y = 0; y < TILE_HEIGHT; ++y) {
                for (int x = 0; x < TILE_WIDTH; ++x) {
                    int srcX = tx * TILE_WIDTH + x;
                    int srcY = ty * TILE_HEIGHT + y;
                    int srcIdx = (srcY * width + srcX) * 4;
                    for (int c = 0; c < 4; ++c) {
                        tileData.push_back(textureData[srcIdx + c]);
                    }
                }
            }

            // Hash the tile data
            size_t tileHash = hashTileData(tileData.data(), tileData.size());

            // If any tile hash doesn't exist in the file, mark the check as failed
            if (tileHash != textureTileHashes[tileIndex]) {
                allTilesExist = false;
                break;
            }
        }
    }

    inFile.close();
    return allTilesExist;
}

int main() {
    // Load texture (replace with your actual texture file)
    int width, height;
    unsigned char* textureData = loadTexture("texture.png", width, height);
    if (!textureData) return -1;

    // Pack the texture tiles into a single file
    saveTextureTiles(textureData, width, height, "packed_texture.bin");

    // Check if the texture tiles exist in the packed file
    bool exists = textureTilesExist("packed_texture.bin", textureData, width, height);
    std::cout << "Tiles " << (exists ? "exist" : "do not exist") << " in the packed file." << std::endl;

    stbi_image_free(textureData); // Don't forget to free the texture data
    return 0;
}

*/