/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "test_suite.h"

#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <credentials/keys/signature_params.h>

static struct {
	chunk_t aid;
	rsa_pss_params_t params;
} rsa_pss_parse_tests[] = {
	/* from RFC 7427, no parameters (empty sequence) */
	{ chunk_from_chars(0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x00),
	  { .hash = HASH_SHA1, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA1, }},
	/* from RFC 7427, default parameters (SHA-1), would actually not be sent
	 * like this, as corrected in errata */
	{ chunk_from_chars(0x30,0x3e,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x31,0xa0,
					   0x0b,0x30,0x09,0x06,0x05,0x2b,0x0e,0x03,0x02,0x1a,0x05,0x00,0xa1,0x18,0x30,0x16,
					   0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,0x30,0x09,0x06,0x05,0x2b,
					   0x0e,0x03,0x02,0x1a,0x05,0x00,0xa2,0x03,0x02,0x01,0x14,0xa3,0x03,0x02,0x01,0x01),
	  { .hash = HASH_SHA1, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA1, }},
	/* from RFC 7427, SHA-256 */
	{ chunk_from_chars(0x30,0x46,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x39,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,0x30,
					   0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0xa2,0x03,
					   0x02,0x01,0x20,0xa3,0x03,0x02,0x01,0x01),
	  { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, }},
	/* from RFC 7427, SHA-256 (errata, without trailer, with len corrections) */
	{ chunk_from_chars(0x30,0x41,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x34,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,0x30,
					   0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0xa2,0x03,
					   0x02,0x01,0x20),
	  { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, }},
	/* SHA-512 */
	{ chunk_from_chars(0x30,0x41,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x34,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,0x05,0x00,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,0x30,
					   0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,0x05,0x00,0xa2,0x03,
					   0x02,0x01,0x40),
	  { .hash = HASH_SHA512, .mgf1_hash = HASH_SHA512, .salt_len = HASH_SIZE_SHA512, }},
	/* SHA-256, no salt */
	{ chunk_from_chars(0x30,0x41,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x34,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,0x30,
					   0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0xa2,0x03,
					   0x02,0x01,0x00),
		{ .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = 0, }},
	/* only hash specified */
	{ chunk_from_chars(0x30,0x1e,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x11,
					   0xa0,0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,
					   0x05,0x00),
		{ .hash = HASH_SHA256, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA1, }},
	/* only mgf specified */
	{ chunk_from_chars(0x30,0x2b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x1e,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,
					   0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00),
		{ .hash = HASH_SHA1, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA1, }},
	/* only salt specified */
	{ chunk_from_chars(0x30,0x12,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x05,0xa2,
					   0x03,0x02,0x01,0x20),
	  { .hash = HASH_SHA1, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA256, }},
};

START_TEST(test_rsa_pss_params_parse)
{
	rsa_pss_params_t parsed;
	chunk_t params;
	int oid;

	oid = asn1_parse_algorithmIdentifier(rsa_pss_parse_tests[_i].aid, 0, &params);
	ck_assert_int_eq(OID_RSASSA_PSS, oid);
	ck_assert(rsa_pss_params_parse(params, 1, &parsed));
	ck_assert_int_eq(rsa_pss_parse_tests[_i].params.hash, parsed.hash);
	ck_assert_int_eq(rsa_pss_parse_tests[_i].params.mgf1_hash, parsed.mgf1_hash);
	ck_assert_int_eq(rsa_pss_parse_tests[_i].params.salt_len, parsed.salt_len);
}
END_TEST

chunk_t rsa_pss_parse_invalid_tests[] = {
	/* unknown hash */
	chunk_from_chars(0x30,0x1e,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x11,
					 0xa0,0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x00,
					 0x05,0x00),
	/* unknown mgf */
	chunk_from_chars(0x30,0x2b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x1e,
					 0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x00,
					 0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00),
	/* unknown mgf-1 hash */
	chunk_from_chars(0x30,0x2b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x1e,
					 0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,
					 0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x00,0x05,0x00),
	/* incorrect trailer */
	chunk_from_chars(0x30,0x12,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x05,
					 0xa3,0x03,0x02,0x01,0x02),
	/* too long trailer */
	chunk_from_chars(0x30,0x13,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x06,
					 0xa3,0x04,0x02,0x02,0x01,0x01),
};

START_TEST(test_rsa_pss_params_parse_invalid)
{
	rsa_pss_params_t parsed;
	chunk_t params;
	int oid;

	oid = asn1_parse_algorithmIdentifier(rsa_pss_parse_invalid_tests[_i], 0, &params);
	ck_assert_int_eq(OID_RSASSA_PSS, oid);
	ck_assert(!rsa_pss_params_parse(params, 1, &parsed));
}
END_TEST

static struct {
	chunk_t aid;
	rsa_pss_params_t params;
} rsa_pss_build_tests[] = {
	/* default parameters -> empty sequence */
	{ chunk_from_chars(0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x00),
		{ .hash = HASH_SHA1, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA1, }},
	/* SHA-256 */
	{ chunk_from_chars(0x30,0x41,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x34,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,0x30,
					   0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0xa2,0x03,
					   0x02,0x01,0x20),
		{ .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = RSA_PSS_SALT_LEN_DEFAULT, }},
	/* default salt length: SHA-1 */
	{ chunk_from_chars(0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x00),
		{ .hash = HASH_SHA1, .mgf1_hash = HASH_SHA1, .salt_len = RSA_PSS_SALT_LEN_DEFAULT, }},
	/* default salt length: SHA-224 */
	{ chunk_from_chars(0x30,0x23,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x16,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x04,0x05,0x00,
					   0xa2,0x03,0x02,0x01,0x1c),
		{ .hash = HASH_SHA224, .mgf1_hash = HASH_SHA1, .salt_len = RSA_PSS_SALT_LEN_DEFAULT, }},
	/* default salt length: SHA-384 */
	{ chunk_from_chars(0x30,0x23,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x16,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x02,0x05,0x00,
					   0xa2,0x03,0x02,0x01,0x30),
		{ .hash = HASH_SHA384, .mgf1_hash = HASH_SHA1, .salt_len = RSA_PSS_SALT_LEN_DEFAULT, }},
	/* SHA-512 */
	{ chunk_from_chars(0x30,0x41,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x34,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,0x05,0x00,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,0x30,
					   0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,0x05,0x00,0xa2,0x03,
					   0x02,0x01,0x40),
	  { .hash = HASH_SHA512, .mgf1_hash = HASH_SHA512, .salt_len = RSA_PSS_SALT_LEN_DEFAULT, }},
	/* SHA-256, no salt */
	{ chunk_from_chars(0x30,0x41,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x34,0xa0,
					   0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,0x30,
					   0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0xa2,0x03,
					   0x02,0x01,0x00),
		{ .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = 0, }},
	/* SHA-256, rest default */
	{ chunk_from_chars(0x30,0x1e,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x11,
					   0xa0,0x0f,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,
					   0x05,0x00),
		{ .hash = HASH_SHA256, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA1, }},
	/* MGF1-SHA-256, rest default */
	{ chunk_from_chars(0x30,0x2b,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x1e,
					   0xa1,0x1c,0x30,0x1a,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x08,
					   0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00),
		{ .hash = HASH_SHA1, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA1, }},
	/* only salt specified */
	{ chunk_from_chars(0x30,0x12,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x0a,0x30,0x05,0xa2,
					   0x03,0x02,0x01,0x20),
	  { .hash = HASH_SHA1, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA256, }},
};

START_TEST(test_rsa_pss_params_build)
{
	chunk_t params, aid;

	ck_assert(rsa_pss_params_build(&rsa_pss_build_tests[_i].params, &params));
	aid = asn1_wrap(ASN1_SEQUENCE, "mm", asn1_build_known_oid(OID_RSASSA_PSS),
					params);
	ck_assert_chunk_eq(rsa_pss_build_tests[_i].aid, aid);
	chunk_free(&aid);
}
END_TEST

rsa_pss_params_t rsa_pss_build_invalid_tests[] = {
	/* unknown hash */
	{ .hash = HASH_UNKNOWN, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA1, },
	/* invalid mgf */
	{ .hash = HASH_SHA256, .mgf1_hash = HASH_UNKNOWN, .salt_len = HASH_SIZE_SHA256, },
};

START_TEST(test_rsa_pss_params_build_invalid)
{
	chunk_t params;

	ck_assert(!rsa_pss_params_build(&rsa_pss_build_invalid_tests[_i], &params));
}
END_TEST

static rsa_pss_params_t rsa_pss_params_sha1 = { .hash = HASH_SHA1, .mgf1_hash = HASH_SHA1, .salt_len = HASH_SIZE_SHA1, };
static rsa_pss_params_t rsa_pss_params_sha256 = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = HASH_SIZE_SHA256, };
static rsa_pss_params_t rsa_pss_params_sha256_mgf1 = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA512, .salt_len = HASH_SIZE_SHA256, };
static rsa_pss_params_t rsa_pss_params_sha256_salt = { .hash = HASH_SHA256, .mgf1_hash = HASH_SHA256, .salt_len = 10, };

static struct {
	bool equal;
	bool complies;
	signature_params_t a;
	signature_params_t b;
} params_compare_tests[] = {
	{ TRUE, TRUE, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256, }, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256, }, },
	{ FALSE, FALSE, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA1, }, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256, }, },
	{ TRUE, TRUE, { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256 },
				  { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256 }, },
	{ FALSE, FALSE, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256, .params = &rsa_pss_params_sha256 },
					{ .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256, .params = &rsa_pss_params_sha256 }, },
	{ FALSE, FALSE, { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256 },
					{ .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256_mgf1 }, },
	{ FALSE, TRUE, { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256 },
				   { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256_salt }, },
	{ FALSE, FALSE, { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha1 },
					{ .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256 }, },
	{ FALSE, FALSE, { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256 },
					{ .scheme = SIGN_RSA_EMSA_PSS, }, },
};

START_TEST(test_params_compare)
{
	bool res;

	res = signature_params_equal(&params_compare_tests[_i].a,
								 &params_compare_tests[_i].b);
	ck_assert(res == params_compare_tests[_i].equal);
	res = signature_params_comply(&params_compare_tests[_i].a,
								  &params_compare_tests[_i].b);
	ck_assert(res == params_compare_tests[_i].complies);
	res = signature_params_comply(&params_compare_tests[_i].b,
								  &params_compare_tests[_i].a);
	ck_assert(res == params_compare_tests[_i].complies);
}
END_TEST

START_TEST(test_params_compare_null)
{
	ck_assert(signature_params_equal(NULL, NULL));
	ck_assert(!signature_params_equal(&params_compare_tests[0].a, NULL));
	ck_assert(!signature_params_equal(NULL, &params_compare_tests[0].a));
}
END_TEST

static struct {
	signature_params_t src;
	signature_params_t res;
} params_clone_tests[] = {
	{ { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256, }, { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256, }, },
	{ { .scheme = SIGN_RSA_EMSA_PSS }, { .scheme = SIGN_RSA_EMSA_PSS }, },
	{ { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256 },
	  { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256 }, },
	{ { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256_salt },
	  { .scheme = SIGN_RSA_EMSA_PSS, .params = &rsa_pss_params_sha256_salt }, },
	{ { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256, .params = &rsa_pss_params_sha256 },
	  { .scheme = SIGN_RSA_EMSA_PKCS1_SHA2_256 }, },
};

START_TEST(test_params_clone)
{
	signature_params_t *clone = NULL;

	clone = signature_params_clone(&params_clone_tests[_i].src);
	ck_assert(signature_params_equal(clone, &params_clone_tests[_i].res));
	signature_params_destroy(clone);
}
END_TEST

START_TEST(test_params_clone_null)
{
	signature_params_t *clone = NULL;

	clone = signature_params_clone(clone);
	ck_assert(!clone);
	signature_params_destroy(clone);
}
END_TEST

START_TEST(test_params_clear)
{
	signature_params_t *clone;

	clone = signature_params_clone(&params_clone_tests[_i].src);
	signature_params_clear(clone);
	ck_assert_int_eq(clone->scheme, SIGN_UNKNOWN);
	ck_assert(!clone->params);
	free(clone);
}
END_TEST

START_TEST(test_params_clear_null)
{
	signature_params_t *clone = NULL;

	signature_params_clear(clone);
}
END_TEST

Suite *signature_params_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("signature params");

	tc = tcase_create("rsa/pss parse");
	tcase_add_loop_test(tc, test_rsa_pss_params_parse, 0, countof(rsa_pss_parse_tests));
	tcase_add_loop_test(tc, test_rsa_pss_params_parse_invalid, 0, countof(rsa_pss_parse_invalid_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("rsa/pss build");
	tcase_add_loop_test(tc, test_rsa_pss_params_build, 0, countof(rsa_pss_build_tests));
	tcase_add_loop_test(tc, test_rsa_pss_params_build_invalid, 0, countof(rsa_pss_build_invalid_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("params compare");
	tcase_add_loop_test(tc, test_params_compare, 0, countof(params_compare_tests));
	tcase_add_test(tc, test_params_compare_null);
	suite_add_tcase(s, tc);

	tc = tcase_create("params clone");
	tcase_add_loop_test(tc, test_params_clone, 0, countof(params_clone_tests));
	tcase_add_test(tc, test_params_clone_null);
	suite_add_tcase(s, tc);

	tc = tcase_create("params clear");
	tcase_add_loop_test(tc, test_params_clear, 0, countof(params_clone_tests));
	tcase_add_test(tc, test_params_clear_null);
	suite_add_tcase(s, tc);

	return s;
}
