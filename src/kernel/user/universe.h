//! @file universe.h
//! @brief File containing declarations related to universes

#pragma once

#include <user/cookie.h>
#include <user/object.h>
#include <user/status.h>

//! @brief Universe. Addressable collection of object references
struct user_universe;

//! @brief Create new universe
//! @param universe Buffer to store reference to the new universe in
//! @return API status
int user_universe_create(struct user_universe **universe);

//! @brief Allocate cell for new reference
//! @param universe Universe to allocate cell in
//! @param ref Reference to store
//! @param cell Buffer to store cell index in
//! @return API status
int user_universe_move_in(struct user_universe *universe, struct user_ref ref, size_t *cell);

//! @brief Allocate two cells for two references (neither of them can be universes)
//! @param universe Universe to allocate cells in
//! @param refs Array of two references to store
//! @param cells Array of two cell indices to store results in
//! @return API status
int user_universe_move_in_pair(struct user_universe *universe, struct user_ref *refs,
                               size_t *cells);

//! @brief Drop reference at index
//! @param universe Universe to drop reference from
//! @param cell Index of cell
//! @param cookie Entry cookie
//! @return API status
int user_universe_drop_cell(struct user_universe *universe, size_t cell,
                            struct user_entry_cookie *cookie);

//! @brief Borrow reference to the object at index
//! @param universe Universe to borrow reference from
//! @param cell Index of cell with the reference
//! @param buf Buffer to store borrowed reference in
//! @return API status
int user_universe_borrow_out(struct user_universe *universe, size_t cell, struct user_ref *buf);

//! @brief Move reference to the object at index
//! @param universe Universe to move reference from
//! @param cell Index of cell with the references
//! @param buf Buffer to store reference in
//! @return API status
int user_universe_move_out(struct user_universe *universe, size_t cell, struct user_ref *buf);

//! @brief Move reference from one universe to the other
//! @param src Source universe
//! @param dst Destination universe
//! @param hsrc Handle in source universe
//! @param hdst Buffer to store handle for destination universe
//! @param cookie Entry cookie
//! @return API status
int user_universe_move_across(struct user_universe *src, struct user_universe *dst, size_t hsrc,
                              size_t *hdst, struct user_entry_cookie *cookie);

//! @brief Borrow reference from one universe to the other
//! @param src Source universe
//! @param dst Destination universe
//! @param hsrc Handle in source universe
//! @param hdst Buffer to store handle for destination universe
//! @param cookie Entry cookie
//! @return API status
int user_universe_borrow_across(struct user_universe *src, struct user_universe *dst, size_t hsrc,
                                size_t *hdst, struct user_entry_cookie *cookie);

//! @brief Unpin reference
//! @param universe Pointer to the universe
//! @param handle Handle to unpin
//! @param cookie Entry cookie
//! @return API status
int user_universe_unpin(struct user_universe *universe, size_t handle,
                        struct user_entry_cookie *cookie);

//! @brief Pin reference
//! @param universe Pointer to the universe
//! @param handle Handle to unpin
//! @param cookie Entry cookie
//! @return API status
int user_universe_pin(struct user_universe *universe, size_t handle,
                      struct user_entry_cookie *cookie);

//! @brief Unpin reference from group
//! @param universe Pointer to the universe
//! @param handle Handle to unpin
//! @param entry Entry cookie
//! @param group Group cookie
//! @return API status
int user_universe_unpin_from_group(struct user_universe *universe, size_t handle,
                                   struct user_entry_cookie *entry,
                                   struct user_group_cookie *group);

//! @brief Pin reference to group
//! @param universe Pointer to the universe
//! @param handle Handle to unpin
//! @param cookie Entry cookie
//! @param entry Entry cookie
//! @param group Group cookie
//! @return API status
int user_universe_pin_to_group(struct user_universe *universe, size_t handle,
                               struct user_entry_cookie *entry, struct user_group_cookie *group);

//! @brief Fork universe (create new universe and copy all accessible handles)
//! @param src Pointer to the universe
//! @param dst Buffer to store reference to the new universe in
//! @param cookie Entry cookie
//! @return API status
int user_universe_fork(struct user_universe *src, struct user_universe **dst,
                       struct user_entry_cookie *cookie);
