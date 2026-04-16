/* SPDX-License-Identifier: GPL-2.0+ OR MIT */
/*
 * Copyright (C) 2025 Tencent, Inc.
 *             http://www.tencent.com/
 */
#ifndef __EROFS_OCI_H
#define __EROFS_OCI_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CURL;
struct erofs_importer;

/*
 * struct ocierofs_config - OCI configuration structure
 * @image_ref: OCI image reference (e.g., "ubuntu:latest", "myregistry.com/app:v1.0")
 * @platform: target platform in "os/arch" format (e.g., "linux/amd64")
 * @username: username for authentication (optional)
 * @password: password for authentication (optional)
 * @blob_digest: specific blob digest to extract (NULL for all layers)
 * @layer_index: specific layer index to extract (negative for all layers)
 * @insecure: use HTTP for registry communication (optional)
 *
 * Configuration structure for OCI image parameters including registry
 * location, image identification, platform specification, and authentication
 * credentials.
 */
struct ocierofs_config {
	char *image_ref;
	char *platform;
	char *username;
	char *password;
	char *blob_digest;
	int layer_index;
	char *tarindex_path;
	char *zinfo_path;
	bool insecure;
};

/*
 * struct ocierofs_build_result - result of building from OCI image
 * @erofs_layer_paths: array of temporary EROFS layer file paths
 * @erofs_layer_count: number of entries in @erofs_layer_paths
 *
 * When the OCI image uses application/vnd.erofs.layer.v1 layers in index-only
 * mode, mkfs will rebuild from these temporary EROFS images. The caller is
 * responsible for unlinking and freeing @erofs_layer_paths after building its
 * own rebuild source list.
 */
struct ocierofs_build_result {
	char **erofs_layer_paths;
	unsigned int erofs_layer_count;
};

struct ocierofs_layer_info {
	char *digest;
	char *media_type;
	u64 size;
};

struct ocierofs_ctx {
	struct CURL *curl;
	char *auth_header;
	bool using_basic;
	char *registry;
	char *repository;
	char *platform;
	char *tag;
	char *manifest_digest;
	struct ocierofs_layer_info **layers;
	char *blob_digest;
	int layer_count;
	const char *schema;
};

struct ocierofs_iostream {
	struct ocierofs_ctx *ctx;
	u64 offset;
};

/*
 * ocierofs_build_trees - Build from OCI image and/or return EROFS layer paths
 * @importer: erofs importer to populate for tar-based layers
 * @cfg:      oci configuration
 * @res:      result of building from OCI image
 *
 * For tar-based layers, leaves @res empty and populates the importer directly.
 * For EROFS naive layers (application/vnd.erofs.layer.v1) in index-only mode,
 * downloads each layer blob into a temporary EROFS file and returns their
 * paths via @res, for the caller to consume via the rebuild path.
 */
int ocierofs_build_trees(struct erofs_importer *importer,
			const struct ocierofs_config *cfg,
			struct ocierofs_build_result *res);
int ocierofs_ctx_init(struct ocierofs_ctx *ctx,
		      const struct ocierofs_config *cfg);
void ocierofs_ctx_cleanup(struct ocierofs_ctx *ctx);
int ocierofs_io_open(struct erofs_vfile *vf, const struct ocierofs_config *cfg);

char *ocierofs_encode_userpass(const char *username, const char *password);
int ocierofs_decode_userpass(const char *b64, char **out_user, char **out_pass);
const char *ocierofs_get_platform_spec(void);

#ifdef __cplusplus
}
#endif

#endif /* __EROFS_OCI_H */
