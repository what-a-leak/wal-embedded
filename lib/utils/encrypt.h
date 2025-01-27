#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stdint.h>
#include <stddef.h>

#ifndef DISABLE_SECURITY

#define AES_KEY_SIZE 16       // AES-128 = 16 bytes
#define IV_STATIC_SIZE 14     // Static portion of IV
#define IV_DYNAMIC_SIZE 2     // Dynamic portion of IV
#define BLOCK_SIZE 16         // AES block size (128 bits)
#define MAX_PAYLOAD_SIZE 256  // Maximum LoRa payload size

extern const uint8_t shared_key[AES_KEY_SIZE];
extern const uint8_t static_iv[IV_STATIC_SIZE];

/**
 * Encrypts a message using AES-CBC with a static/dynamic IV approach.
 * 
 * @param message         The input plaintext message.
 * @param message_len     The length of the plaintext message.
 * @param encrypted_data  The output encrypted data (dynamic IV + ciphertext).
 * @param encrypted_len   Pointer to store the length of the encrypted data.
 * @return 0 on success, -1 if the payload exceeds the maximum allowed size.
 */
int encrypt_message(const char *message, size_t message_len, uint8_t *encrypted_data, size_t *encrypted_len);

/**
 * Decrypts an encrypted payload using AES-CBC with a static/dynamic IV approach.
 * 
 * @param encrypted_data  The input encrypted data (dynamic IV + ciphertext).
 * @param encrypted_len   The length of the encrypted data.
 * @param decrypted_data  The output plaintext message.
 * @param decrypted_len   Pointer to store the length of the decrypted message.
 * @return 0 on success, -1 on failure.
 */
int decrypt_message(const uint8_t *encrypted_data, size_t encrypted_len, char *decrypted_data, size_t *decrypted_len);

#endif // DISABLE_SECURITY
#endif // ENCRYPT_H
