#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stdint.h>
#include <stddef.h>

#define AES_KEY_SIZE 16 // AES-128 = 16 bytes
#define BLOCK_SIZE 16   // 128 bits
#define MAX_PAYLOAD_SIZE 256

extern const uint8_t shared_key[AES_KEY_SIZE];

/**
 * Encrypts a message using AES-CBC.
 * 
 * @param message         The input plaintext message.
 * @param message_len     The length of the plaintext message.
 * @param encrypted_data  The output encrypted data (IV + ciphertext).
 * @param encrypted_len   Pointer to store the length of the encrypted data.
 * @return 0 on success, -1 if the payload exceeds the maximum allowed size.
 */
int encrypt_message(const char *message, size_t message_len, uint8_t *encrypted_data, size_t *encrypted_len);

/**
 * Decrypts an encrypted payload using AES-CBC.
 * 
 * @param encrypted_data  The input encrypted data (IV + ciphertext).
 * @param encrypted_len   The length of the encrypted data.
 * @param decrypted_data  The output plaintext message.
 * @param decrypted_len   Pointer to store the length of the decrypted message.
 * @return 0 on success, -1 on failure.
 */
int decrypt_message(const uint8_t *encrypted_data, size_t encrypted_len, char *decrypted_data, size_t *decrypted_len);

#endif // ENCRYPT_H
