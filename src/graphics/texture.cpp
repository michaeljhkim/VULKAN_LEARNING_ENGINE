#include "texture.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <unordered_set>

/*
    constructor
*/

Texture::Texture(VulkanDevice &vulkanDevice, std::string name) : 
                                        vulkanDevice(vulkanDevice), name(name), type(aiTextureType_NONE) {
    generate();
}

// initialize with image path and type
Texture::Texture(VulkanDevice &vulkanDevice, std::string dir, std::string path, aiTextureType type) : 
                                        vulkanDevice(vulkanDevice), dir(dir), path(path), type(type) {
    generate();
}

// generate texture id
void Texture::generate() {
    textureHash = hashData(pixels, texWidth * texHeight * 4);
}








// Hash data (e.g., tile or texture data)
size_t hashData(const unsigned char* data, size_t dataSize) {
	std::vector<unsigned char> texData(data, data + dataSize);
    std::hash<std::string> hash_fn;
    return hash_fn(std::string(texData.begin(), texData.end()));
}


// Save texture tiles to the file
void Texture::saveTextureTiles(unsigned char* pixels, int texWidth, int texHeight, std::ofstream& outFile) {
    // Write the header
    int tilesX = texWidth / TILE_WIDTH;
    int tilesY = texHeight / TILE_HEIGHT;
    int totalTiles = tilesX * tilesY;

    outFile.write(reinterpret_cast<const char*>(&totalTiles), sizeof(totalTiles));
    outFile.write(reinterpret_cast<const char*>(&TILE_WIDTH), sizeof(TILE_WIDTH));
    outFile.write(reinterpret_cast<const char*>(&TILE_HEIGHT), sizeof(TILE_HEIGHT));
    outFile.write(reinterpret_cast<const char*>(&textureHash), sizeof(textureHash));	// Calculate and write the texture hash

    std::streampos offsetTablePosition = outFile.tellp();								// Offset table placeholder
    outFile.write(reinterpret_cast<const char*>(&offsetTablePosition), sizeof(offsetTablePosition));
    std::vector<unsigned char> tileData(TILE_SIZE);
    std::vector<Tile> tiles;

    // Process tiles
    for (int ty = 0; ty < tilesY; ++ty) {
        for (int tx = 0; tx < tilesX; ++tx) {
            Tile tile;
            tile.offset = static_cast<int>(outFile.tellp());

            // Extract tile pixels into tileData
            for (int y = 0; y < TILE_HEIGHT; ++y) {
                for (int x = 0; x < TILE_WIDTH; ++x) {
                    int srcX = tx * TILE_WIDTH + x;
                    int srcY = ty * TILE_HEIGHT + y;
                    int srcIdx = (srcY * texWidth + srcX) * 4;

                    std::copy_n(&pixels[srcIdx], 4, &tileData[(y * TILE_WIDTH + x) * 4]);
                }
            }

            // Compute hash and write tile data
            tile.hash = hashData(tileData.data(), TILE_SIZE);
            outFile.write(reinterpret_cast<const char*>(tileData.data()), TILE_SIZE);
            tiles.push_back(tile);
        }
    }

    // Write the offset table
    outFile.seekp(offsetTablePosition);
    for (const Tile& tile : tiles) {
        outFile.write(reinterpret_cast<const char*>(&tile.offset), sizeof(tile.offset));
        outFile.write(reinterpret_cast<const char*>(&tile.hash), sizeof(tile.hash));
    }
}



// Pack multiple textures into the output file
void Texture::packTextures(const std::vector<std::string>& texturePaths, const std::string& outputFile, bool flip) {
    //std::ofstream outFile(outputFile, std::ios::binary | std::ios::app);
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    // Global lookup table (load existing data)
    std::vector<TextureMetadata> globalLookupTable = loadGlobalLookupTable(outputFile);

    for (const auto& texturePath : texturePaths) {
        int texWidth, texHeight, texChannels;
        stbi_set_flip_vertically_on_load(flip);
        unsigned char* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        // Validate texture dimensions
        if (!pixels || (texWidth % TILE_WIDTH != 0 || texHeight % TILE_HEIGHT != 0) ) {
            std::cerr << "Failed to load texture: " << texturePath << std::endl;
            std::cerr << "Or Texture dimensions not divisible by tile size: " << texturePath << std::endl;
            stbi_image_free(pixels);
            continue;
        }
		
        // Check for duplicates
        auto it = std::find_if(globalLookupTable.begin(), globalLookupTable.end(),
								[](const TextureMetadata& metadata) {
									return metadata.textureHash == Texture::textureHash;
								});

        if (it != globalLookupTable.end()) {
            std::cout << "Texture already exists in the file, skipping: " << texturePath << std::endl;
            stbi_image_free(pixels);
            continue;
        }

        // Save texture and update metadata
        TextureMetadata metadata;
        metadata.textureHash = textureHash;
        metadata.offset = outFile.tellp();
        metadata.width = texWidth;
        metadata.height = texHeight;
        metadata.tileCount = (texWidth / TILE_WIDTH) * (texHeight / TILE_HEIGHT);

        saveTextureTiles(pixels, texWidth, texHeight, outFile);
        globalLookupTable.push_back(metadata);
        stbi_image_free(pixels);
    }

    // Save updated lookup table
    saveGlobalLookupTable(globalLookupTable, outFile);

    outFile.close();
    std::cout << "Textures packed successfully!" << std::endl;
}




void Texture::saveGlobalLookupTable(const std::vector<TextureMetadata>& globalLookupTable, std::ofstream& outFile) {
    // Write metadata for each texture
    for (const auto& metadata : globalLookupTable) {
        outFile.write(reinterpret_cast<const char*>(&metadata.textureHash), sizeof(metadata.textureHash));
        outFile.write(reinterpret_cast<const char*>(&metadata.offset), sizeof(metadata.offset));
        outFile.write(reinterpret_cast<const char*>(&metadata.width), sizeof(metadata.width));
        outFile.write(reinterpret_cast<const char*>(&metadata.height), sizeof(metadata.height));
        outFile.write(reinterpret_cast<const char*>(&metadata.tileCount), sizeof(metadata.tileCount));
    }

    // Write the number of textures in the lookup table
    int textureCount = globalLookupTable.size();
    outFile.write(reinterpret_cast<const char*>(&textureCount), sizeof(textureCount));
}


std::vector<TextureMetadata> loadGlobalLookupTable(const std::string& outputFile) {
    std::ifstream inFile(outputFile, std::ios::binary);
    std::vector<TextureMetadata> globalLookupTable;
    if (!inFile) {
        std::cerr << "Failed to open file for reading." << std::endl;
        return globalLookupTable; 		// Return an empty table
    }

    // Seek to the end of the file to check if the lookup table exists
    inFile.seekg(0, std::ios::end);
    std::streampos fileSize = inFile.tellg();

    if (fileSize < sizeof(int)) {
        std::cerr << "File is too small to contain a valid lookup table." << std::endl;
        return globalLookupTable;
    }

    // Seek back to where the texture count is stored and Read the number of textures in the table
    inFile.seekg(fileSize - static_cast<std::streamoff>(sizeof(int)), std::ios::beg);
    int textureCount = 0;
    inFile.read(reinterpret_cast<char*>(&textureCount), sizeof(textureCount));

    // Validate the table size
    if (textureCount <= 0) {
        return globalLookupTable;
    }

    // Seek to where the lookup table starts
    std::streampos tableStart = fileSize - static_cast<std::streamoff>( sizeof(int) - (textureCount * sizeof(TextureMetadata)) );
    inFile.seekg(tableStart, std::ios::beg);

    // Read all texture metadata
    for (int i = 0; i < textureCount; ++i) {
        TextureMetadata metadata;
        inFile.read(reinterpret_cast<char*>(&metadata.textureHash), sizeof(metadata.textureHash));
        inFile.read(reinterpret_cast<char*>(&metadata.offset), sizeof(metadata.offset));
        inFile.read(reinterpret_cast<char*>(&metadata.width), sizeof(metadata.width));
        inFile.read(reinterpret_cast<char*>(&metadata.height), sizeof(metadata.height));
        inFile.read(reinterpret_cast<char*>(&metadata.tileCount), sizeof(metadata.tileCount));
        globalLookupTable.push_back(metadata);
    }

    return globalLookupTable;
}


/*
std::vector<Tile> loadTilesForTexture(const std::string& outputFile, size_t textureHash) {
    std::ifstream inFile(outputFile, std::ios::binary);
    std::vector<Tile> tiles;

    if (!inFile) {
        std::cerr << "Failed to open file for reading." << std::endl;
        return tiles; // Return an empty list if the file can't be opened
    }

    // Load the global lookup table
    std::vector<TextureMetadata> globalLookupTable = loadGlobalLookupTable(outputFile);

    // Find the texture metadata for the given textureHash
    auto it = std::find_if(globalLookupTable.begin(), globalLookupTable.end(),
                           [textureHash](const TextureMetadata& metadata) {
                               return metadata.textureHash == textureHash;
                           });

    if (it == globalLookupTable.end()) {
        std::cerr << "Texture with hash " << textureHash << " not found." << std::endl;
        return tiles; // Return empty if texture not found
    }

    // Get the metadata for the texture
    TextureMetadata metadata = *it;

    // Seek to the position where the tiles start in the file
    inFile.seekg(metadata.offset, std::ios::beg);

    // Read the tiles
    for (int i = 0; i < metadata.tileCount; ++i) {
        TileData tile;

        // Assuming TileData has fields such as width, height, and data buffer (adjust according to actual format)
        inFile.read(reinterpret_cast<char*>(&tile.width), sizeof(tile.width));
        inFile.read(reinterpret_cast<char*>(&tile.height), sizeof(tile.height));
        inFile.read(reinterpret_cast<char*>(&tile.data), tile.width * tile.height * sizeof(char));  // Assuming 1 byte per pixel, adjust if necessary

        tiles.push_back(tile);
    }

    return tiles;
}
*/