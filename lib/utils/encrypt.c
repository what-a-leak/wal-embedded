#include "encrypt.h"
#include <string.h>
#include <stdlib.h>
#include "aes.h" // TinyAES library: https://github.com/kokke/tiny-AES-c

const uint8_t shared_key[AES_KEY_SIZE] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

static void generate_random_iv(uint8_t *iv, size_t len) {
    for (size_t i = 0; i < len; i++) {
        iv[i] = rand() % 256;
    }
}

int encrypt_message(const char *message, size_t message_len, uint8_t *encrypted_data, size_t *encrypted_len) {
    if (message_len > MAX_PAYLOAD_SIZE - BLOCK_SIZE) {
        return -1; // Message exceed 256 bytes (LoRa Limitation)
    }

    struct AES_ctx ctx;
    size_t padded_len = ((message_len / BLOCK_SIZE) + 1) * BLOCK_SIZE;

    uint8_t iv[BLOCK_SIZE];
    generate_random_iv(iv, BLOCK_SIZE);
    AES_init_ctx_iv(&ctx, shared_key, iv);

    uint8_t buffer[padded_len];
    memset(buffer, 0, padded_len);
    memcpy(buffer, message, message_len);

    // PKCS#7 padding
    uint8_t padding_value = padded_len - message_len;
    for (size_t i = message_len; i < padded_len; i++) {
        buffer[i] = padding_value;
    }

    AES_CBC_encrypt_buffer(&ctx, buffer, padded_len);

    memcpy(encrypted_data, iv, BLOCK_SIZE); // Prepend IV to the encrypted data
    memcpy(encrypted_data + BLOCK_SIZE, buffer, padded_len);
    *encrypted_len = BLOCK_SIZE + padded_len;

    return 0;
}

int decrypt_message(const uint8_t *encrypted_data, size_t encrypted_len, char *decrypted_data, size_t *decrypted_len) {
    if (encrypted_len < BLOCK_SIZE || (encrypted_len - BLOCK_SIZE) % BLOCK_SIZE != 0) {
        return -1; // Invalid encrypted data size
    }

    struct AES_ctx ctx;
    uint8_t iv[BLOCK_SIZE];
    memcpy(iv, encrypted_data, BLOCK_SIZE);

    size_t ciphertext_len = encrypted_len - BLOCK_SIZE;
    uint8_t buffer[ciphertext_len];
    memcpy(buffer, encrypted_data + BLOCK_SIZE, ciphertext_len);

    AES_init_ctx_iv(&ctx, shared_key, iv);
    AES_CBC_decrypt_buffer(&ctx, buffer, ciphertext_len);

    // Remove PKCS#7 padding
    uint8_t padding_value = buffer[ciphertext_len - 1];
    if (padding_value > BLOCK_SIZE || padding_value > ciphertext_len) {
        return -1; // Invalid padding
    }

    *decrypted_len = ciphertext_len - padding_value;
    memcpy(decrypted_data, buffer, *decrypted_len);

    return 0;
}
