/*
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Niels Dossche <nielsdos@php.net>                            |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "php.h"
#if defined(HAVE_LIBXML) && defined(HAVE_DOM)
#include "php_dom.h"
#include "obj_map.h"
#include "nodelist.h"
#include "html_collection.h"
#include "namespace_compat.h"

typedef struct dom_named_item {
	dom_object *context_intern;
	xmlNodePtr node;
} dom_named_item;

/* https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key */
static dom_named_item dom_html_collection_named_item(zend_string *key, zend_object *zobj)
{
	dom_named_item ret = {NULL, NULL};

	/* 1. If key is the empty string, return null. */
	if (ZSTR_LEN(key) == 0) {
		return ret;
	}

	dom_object *intern = php_dom_obj_from_obj(zobj);
	dom_nnodemap_object *objmap = intern->ptr;

	/* 2. Return the first element in the collection for which at least one of the following is true: */
	xmlNodePtr basep = dom_object_get_node(objmap->baseobj);
	if (basep != NULL && basep->children != NULL) {
		php_dom_obj_map_collection_iter iter = {0};
		iter.candidate = basep->children;
		iter.basep = basep;

		while (true) {
			objmap->handler->collection_named_item_iter(objmap, &iter);
			if (iter.candidate == NULL) {
				break;
			}

			ZEND_ASSERT(iter.candidate->type == XML_ELEMENT_NODE);

			xmlAttrPtr attr;

			/* it has an ID which is key; */
			if ((attr = xmlHasNsProp(iter.candidate, BAD_CAST "id", NULL)) != NULL && dom_compare_value(attr, BAD_CAST ZSTR_VAL(key))) {
				ret.context_intern = objmap->baseobj;
				ret.node = iter.candidate;
				return ret;
			}
			/* it is in the HTML namespace and has a name attribute whose value is key; */
			else if (php_dom_ns_is_fast(iter.candidate, php_dom_ns_is_html_magic_token)) {
				if ((attr = xmlHasNsProp(iter.candidate, BAD_CAST "name", NULL)) != NULL && dom_compare_value(attr, BAD_CAST ZSTR_VAL(key))) {
					ret.context_intern = objmap->baseobj;
					ret.node = iter.candidate;
					return ret;
				}
			}
		}
	}

	return ret;
}

static void dom_html_collection_named_item_into_zval(zval *return_value, zend_string *key, zend_object *zobj)
{
	dom_named_item named_item = dom_html_collection_named_item(key, zobj);
	if (named_item.node != NULL) {
		DOM_RET_OBJ(named_item.node, named_item.context_intern);
	} else {
		RETURN_NULL();
	}
}

PHP_METHOD(Dom_HTMLCollection, namedItem)
{
	zend_string *key;
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(key)
	ZEND_PARSE_PARAMETERS_END();
	dom_html_collection_named_item_into_zval(return_value, key, Z_OBJ_P(ZEND_THIS));
}

zval *dom_html_collection_read_dimension(zend_object *object, zval *offset, int type, zval *rv)
{
	if (UNEXPECTED(!offset)) {
		zend_throw_error(NULL, "Cannot append to %s", ZSTR_VAL(object->ce->name));
		return NULL;
	}

	dom_nodelist_dimension_index index = dom_modern_nodelist_get_index(offset);
	if (UNEXPECTED(index.type == DOM_NODELIST_DIM_ILLEGAL)) {
		zend_illegal_container_offset(object->ce->name, offset, type);
		return NULL;
	}

	if (index.type == DOM_NODELIST_DIM_STRING) {
		dom_html_collection_named_item_into_zval(rv, index.str, object);
	} else {
		ZEND_ASSERT(index.type == DOM_NODELIST_DIM_LONG);
		php_dom_obj_map_get_item_into_zval(php_dom_obj_from_obj(object)->ptr, index.lval, rv);
	}

	return rv;
}

int dom_html_collection_has_dimension(zend_object *object, zval *member, int check_empty)
{
	/* If it exists, it cannot be empty because nodes aren't empty. */
	ZEND_IGNORE_VALUE(check_empty);

	dom_nodelist_dimension_index index = dom_modern_nodelist_get_index(member);
	if (UNEXPECTED(index.type == DOM_NODELIST_DIM_ILLEGAL)) {
		zend_illegal_container_offset(object->ce->name, member, BP_VAR_IS);
		return 0;
	}

	if (index.type == DOM_NODELIST_DIM_STRING) {
		return dom_html_collection_named_item(index.str, object).node != NULL;
	} else {
		ZEND_ASSERT(index.type == DOM_NODELIST_DIM_LONG);
		return index.lval >= 0 && index.lval < php_dom_get_nodelist_length(php_dom_obj_from_obj(object));
	}
}

HashTable *dom_html_collection_get_gc(zend_object *object, zval **table, int *n)
{
	dom_nnodemap_object *objmap = php_dom_obj_from_obj(object)->ptr;

	if (objmap->baseobj) {
		zend_get_gc_buffer *gc_buffer = zend_get_gc_buffer_create();
		zend_get_gc_buffer_add_obj(gc_buffer, &objmap->baseobj->std);
		zend_get_gc_buffer_use(gc_buffer, table, n);

		if (object->properties == NULL && object->ce->default_properties_count == 0) {
			return NULL;
		} else {
			return zend_std_get_properties(object);
		}
	} else {
		return zend_std_get_gc(object, table, n);
	}
}

#endif
