/*******************************************************************************
*   (c) 2018 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/
#include "common.h"

#include <lib/tx_display.h>
#include <lib/parser.h>

#include <gtest/gtest.h>
#include "common.h"

void setup_context(parsed_json_t *parsed_json,
                   unsigned short screen_size,
                   const char *transaction) {
    parsing_context_t context;
    context.parsed_tx = parsed_json;
    context.max_chars_per_key_line = screen_size;
    context.max_chars_per_value_line = screen_size;
    context.tx = transaction;
    set_parsing_context(context);
}

const char *parse_tx(parsed_json_t *parsed_json, const char *tx, uint16_t screen_size) {
    const char *err = json_parse(parsed_json, tx);
    setup_context(parsed_json, screen_size, tx);
    return err;
}

std::string get_pages(const char *input_json, int screen_size) {
    parsed_json_t parsed_json;
    const char *err = parse_tx(&parsed_json, input_json, screen_size);
    if (err != nullptr) {
        return std::string(err);
    }
    setup_context(&parsed_json, screen_size, input_json);

    tx_display_index_root();

    std::ostringstream out;
    char key[screen_size];
    char val[screen_size];
    int num_pages = 1000;

    out << std::endl;
    for (int16_t idx = 0; idx < num_pages; idx++) {
        int16_t num_chunks = 1;
        int16_t chunk_index = 0;

        while (chunk_index < num_chunks) {
            INIT_QUERY_CONTEXT(key, sizeof(key), val, sizeof(val), chunk_index, 2);
            tx_ctx.query.item_index = idx;
            num_chunks = tx_traverse(0);

            if (num_chunks > 0) {
                out << "[" << idx << "] "
                    << chunk_index + 1 << "/"
                    << num_chunks << " | "
                    << key << " : "
                    << val << std::endl;
            } else {
                out << "-----------";
                return out.str();
            }
            chunk_index++;
        }
    }

    return out.str();
}

std::string get_display_pages(const char *input_json, int screen_size, bool make_friendly) {
    parser_context_t ctx;

    parser_error_t err = parser_parse(&ctx, (uint8_t *) input_json, strlen(input_json));

    if (err != parser_ok) {
        const char *errMessage = parser_getErrorDescription(err);
        return std::string(errMessage);
    }

    std::ostringstream out;
    char key[screen_size];
    char value[screen_size];

    // CHECK DISPLAY CACHE
    auto display_cache = tx_display_cache();
    out << std::endl;
    for (int i = 0; i < sizeof(display_cache->num_subpages); i++) {
        out << "[" << i << "]" << (int) display_cache->num_subpages[i] << std::endl;
    }

    // RETRIEVE PAGES
    int8_t num_pages = parser_getNumItems(&ctx);
    // Get raw items
    for (uint8_t displayIdx = 0; displayIdx < num_pages + 1; displayIdx++) {

        uint8_t pageIndex = 0;
        uint8_t pageCount = 1;

        while (pageIndex < pageCount) {
            err = parser_getItem(&ctx,
                                 displayIdx,
                                 key, sizeof(key),
                                 value, sizeof(value),
                                 pageIndex, &pageCount);

            if (pageCount > 0 && err == parser_ok) {
                out << "[" << (int) displayIdx << "] ";
                out << (int) (pageIndex + 1) << "/" << (int) pageCount << " | ";
                out << key;
                if (strlen(value) > 0) {
                    out << " : " << value;
                } else {
                    out << " :";
                }
                out << std::endl;
            } else {
                out << "----------- " << (int) num_pages;
                return out.str();
            }
            pageIndex++;
        }
    }

    return out.str();
}
