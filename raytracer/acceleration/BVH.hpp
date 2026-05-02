#pragma once

/**
 * BVH.hpp
 *
 * Bounding Volume Hierarchy acceleration structure.
 *
 * A BVH organises all scene geometry into a binary tree.
 * Each node stores an axis-aligned bounding box (AABB) that
 * contains all geometry beneath it. Ray traversal skips entire
 * subtrees when the ray misses a node's bounding box.
 *
 * Build strategy: recursive median split along the longest axis.
 * This gives O(log N) average-case intersection instead of O(N).
 *
 * USE_ACCEL flag:
 *   Compile with -DUSE_ACCEL to enable the BVH.
 *   Without this flag, World falls back to brute-force intersection.
 *   See README for full instructions.
 */

#include "Acceleration.hpp"
#include "../utilities/BBox.hpp"
#include "../utilities/Constants.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Ray.hpp"
#include "../geometry/Geometry.hpp"

#include <vector>
#include <algorithm>
#include <memory>

// ---------------------------------------------------------------------------
// BVH node: either a leaf (holds geometry) or an internal node (holds children)
// ---------------------------------------------------------------------------
struct BVHNode {

    BBox                        bbox;           // bounding box of this node
    std::unique_ptr<BVHNode>    left;           // left child (null if leaf)
    std::unique_ptr<BVHNode>    right;          // right child (null if leaf)
    std::vector<Geometry*>      objects;        // only populated in leaf nodes

    // Is this node a leaf?
    bool is_leaf() const {
        return (left == nullptr && right == nullptr);
    }
};


// ---------------------------------------------------------------------------
// BVH class
// ---------------------------------------------------------------------------
class BVH : public Acceleration {
private:

    std::unique_ptr<BVHNode> root;  // root of the BVH tree

    // Maximum objects per leaf before we stop splitting.
    static const int MAX_LEAF_SIZE = 4;

public:

    BVH() = default;
    virtual ~BVH() = default;

    // Build the BVH from a flat list of objects.
    virtual void build(const std::vector<Geometry*>& objects) override;

    // Test a ray against the BVH. Returns closest hit.
    virtual ShadeInfo hit(const Ray& ray, const World& world) const override;

    // Test a shadow ray: returns true if anything blocks it before max_distance.
    virtual bool shadow_hit(const Ray& ray, float max_distance) const override;

private:

    // Recursively build a subtree for the given list of objects.
    std::unique_ptr<BVHNode> build_node(std::vector<Geometry*>& objects,
                                        int start, int end);

    // Recursively test a ray against a node and its children.
    void hit_node(const BVHNode* node,
                  const Ray& ray,
                  float& closest_t,
                  ShadeInfo& closest_info,
                  const World& world) const;

    // Recursively test a shadow ray against a node.
    bool shadow_hit_node(const BVHNode* node,
                         const Ray& ray,
                         float max_distance) const;
};