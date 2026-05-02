/**
 * BVH.cpp
 *
 * Implementation of the Bounding Volume Hierarchy.
 */

#include "BVH.hpp"
#include "../world/World.hpp"
#include "../utilities/ShadeInfo.hpp"

#include <limits>


// ---------------------------------------------------------------------------
// build()
//   Entry point. Builds the full BVH tree from the world's geometry list.
// ---------------------------------------------------------------------------
void BVH::build(const std::vector<Geometry*>& objects) {

    // Make a mutable copy so we can sort it during tree construction.
    std::vector<Geometry*> mutable_objects = objects;

    int start = 0;
    int end   = static_cast<int>(mutable_objects.size());

    root = build_node(mutable_objects, start, end);
}


// ---------------------------------------------------------------------------
// build_node()
//   Recursively build a subtree for objects[start..end).
// ---------------------------------------------------------------------------
std::unique_ptr<BVHNode> BVH::build_node(std::vector<Geometry*>& objects,
                                          int start, int end) {

    std::unique_ptr<BVHNode> node = std::make_unique<BVHNode>();

    int count = end - start;

    // Compute the bounding box of all objects in this range.
    BBox combined = objects[start]->getBBox();
    for (int i = start + 1; i < end; i++) {
        combined.extend(objects[i]->getBBox());
    }
    node->bbox = combined;

    // If small enough, make this a leaf node.
    if (count <= MAX_LEAF_SIZE) {
        for (int i = start; i < end; i++) {
            node->objects.push_back(objects[i]);
        }
        return node;
    }

    // Find the longest axis of the bounding box to split along.
    float x_span = combined.pmax.x - combined.pmin.x;
    float y_span = combined.pmax.y - combined.pmin.y;
    float z_span = combined.pmax.z - combined.pmin.z;

    int split_axis = 0;     // 0 = X, 1 = Y, 2 = Z

    if (y_span > x_span && y_span > z_span) {
        split_axis = 1;
    }
    else if (z_span > x_span && z_span > y_span) {
        split_axis = 2;
    }

    // Sort objects by the centre of their bounding box along the split axis.
    int mid = (start + end) / 2;

    if (split_axis == 0) {
        std::nth_element(
            objects.begin() + start,
            objects.begin() + mid,
            objects.begin() + end,
            [](Geometry* a, Geometry* b) {
                BBox ba = a->getBBox();
                BBox bb = b->getBBox();
                float ca = (ba.pmin.x + ba.pmax.x) * 0.5f;
                float cb = (bb.pmin.x + bb.pmax.x) * 0.5f;
                return ca < cb;
            }
        );
    }
    else if (split_axis == 1) {
        std::nth_element(
            objects.begin() + start,
            objects.begin() + mid,
            objects.begin() + end,
            [](Geometry* a, Geometry* b) {
                BBox ba = a->getBBox();
                BBox bb = b->getBBox();
                float ca = (ba.pmin.y + ba.pmax.y) * 0.5f;
                float cb = (bb.pmin.y + bb.pmax.y) * 0.5f;
                return ca < cb;
            }
        );
    }
    else {
        std::nth_element(
            objects.begin() + start,
            objects.begin() + mid,
            objects.begin() + end,
            [](Geometry* a, Geometry* b) {
                BBox ba = a->getBBox();
                BBox bb = b->getBBox();
                float ca = (ba.pmin.z + ba.pmax.z) * 0.5f;
                float cb = (bb.pmin.z + bb.pmax.z) * 0.5f;
                return ca < cb;
            }
        );
    }

    // Recursively build left and right subtrees.
    node->left  = build_node(objects, start, mid);
    node->right = build_node(objects, mid,   end);

    return node;
}


// ---------------------------------------------------------------------------
// hit()
//   Test a primary ray against the BVH.
// ---------------------------------------------------------------------------
ShadeInfo BVH::hit(const Ray& ray, const World& world) const {

    ShadeInfo closest(world);
    float closest_t = kHugeValue;

    if (root != nullptr) {
        hit_node(root.get(), ray, closest_t, closest, world);
    }

    return closest;
}


// ---------------------------------------------------------------------------
// hit_node()
//   Recursively test a ray against a node.
// ---------------------------------------------------------------------------
void BVH::hit_node(const BVHNode* node,
                   const Ray& ray,
                   float& closest_t,
                   ShadeInfo& closest_info,
                   const World& world) const {

    // Test the ray against this node's bounding box first.
    float t_enter = 0.0f;
    float t_exit  = 0.0f;
    bool  bbox_hit = node->bbox.hit(ray, t_enter, t_exit);

    // If the ray misses the bounding box, skip this entire subtree.
    if (!bbox_hit) {
        return;
    }

    // If the bounding box hit is further than what we already found, skip.
    if (t_enter > closest_t) {
        return;
    }

    // Leaf node: test ray against each object directly.
    if (node->is_leaf()) {
        for (int i = 0; i < (int)node->objects.size(); i++) {
            float      object_t = kHugeValue;
            ShadeInfo  object_info(world);

            bool did_hit = node->objects[i]->hit(ray, object_t, object_info);

            if (did_hit) {
                if (object_t < closest_t) {
                    closest_t    = object_t;
                    closest_info = object_info;
                }
            }
        }
        return;
    }

    // Internal node: recurse into both children.
    if (node->left != nullptr) {
        hit_node(node->left.get(), ray, closest_t, closest_info, world);
    }

    if (node->right != nullptr) {
        hit_node(node->right.get(), ray, closest_t, closest_info, world);
    }
}


// ---------------------------------------------------------------------------
// shadow_hit()
//   Test a shadow ray against the BVH.
//   Returns true as soon as any object is hit before max_distance.
// ---------------------------------------------------------------------------
bool BVH::shadow_hit(const Ray& ray, float max_distance) const {

    if (root == nullptr) {
        return false;
    }

    return shadow_hit_node(root.get(), ray, max_distance);
}


// ---------------------------------------------------------------------------
// shadow_hit_node()
//   Recursively test a shadow ray against a node.
// ---------------------------------------------------------------------------
bool BVH::shadow_hit_node(const BVHNode* node,
                           const Ray& ray,
                           float max_distance) const {

    // Test the bounding box first.
    float t_enter = 0.0f;
    float t_exit  = 0.0f;
    bool  bbox_hit = node->bbox.hit(ray, t_enter, t_exit);

    if (!bbox_hit) {
        return false;
    }

    // If the box is further than the light, no shadow here.
    if (t_enter > max_distance) {
        return false;
    }

    // Leaf node: test each object.
    if (node->is_leaf()) {
        for (int i = 0; i < (int)node->objects.size(); i++) {
            float t = kHugeValue;

            bool did_hit = node->objects[i]->shadow_hit(ray, t);

            if (did_hit) {
                if (t < max_distance) {
                    return true;    // something blocks the light
                }
            }
        }
        return false;
    }

    // Internal node: check both children.
    // Return true as soon as either child finds a blocker.
    if (node->left != nullptr) {
        bool blocked = shadow_hit_node(node->left.get(), ray, max_distance);
        if (blocked) {
            return true;
        }
    }

    if (node->right != nullptr) {
        bool blocked = shadow_hit_node(node->right.get(), ray, max_distance);
        if (blocked) {
            return true;
        }
    }

    return false;
}