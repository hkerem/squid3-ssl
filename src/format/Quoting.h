#ifndef _SQUID_FORMAT_QUOTING_H
#define _SQUID_FORMAT_QUOTING_H

namespace Format
{

/// Safely URL-encode a username.
/// Accepts NULL or empty strings.
extern char * QuoteUrlEncodeUsername(const char *name);

/** URL-style encoding on a MIME headers blob.
 * May accept NULL or empty strings.
 * \return A dynamically allocated string. recipient is responsible for free()'ing
 */
extern char *QuoteMimeBlob(const char *header);

}; // namespace Format

#endif /* _SQUID_FORMAT_QUOTING_H */
