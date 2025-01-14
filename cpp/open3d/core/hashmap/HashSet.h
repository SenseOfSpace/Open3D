// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018-2021 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#pragma once

#include "open3d/core/Dtype.h"
#include "open3d/core/Tensor.h"
#include "open3d/core/hashmap/HashBackendBuffer.h"
#include "open3d/core/hashmap/HashMap.h"

namespace open3d {
namespace core {

class HashSet {
public:
    /// Initialize a hash set given a key dtype and element shape.
    HashSet(int64_t init_capacity,
            const Dtype& key_dtype,
            const SizeVector& key_element_shape,
            const Device& device,
            const HashBackendType& backend = HashBackendType::Default);

    /// Default destructor.
    ~HashSet() = default;

    /// Rehash the internal hash map with the given number of buckets.
    void Rehash(int64_t buckets);

    /// Parallel insert arrays of keys and values in Tensors.
    /// The output_buf_indices and output_masks will be overwritten, to be used
    /// with tensor buffers.
    /// output_buf_indices stores buffer indices that access buffer tensors
    /// obtained from GetKeyTensor() via advanced indexing.
    /// NOTE: output_buf_indices are stored in Int32. A conversion to Int64 is
    /// required for further indexing.
    /// output_masks stores if the insertion is
    /// a success or failure (key already exists).
    void Insert(const Tensor& input_keys,
                Tensor& output_buf_indices,
                Tensor& output_masks);

    /// Parallel find an array of keys in Tensor.
    /// The roles of output_buf_indices is the same as Insert.
    /// output_masks stores if the finding is a success or failure (key
    /// not found).
    void Find(const Tensor& input_keys,
              Tensor& output_buf_indices,
              Tensor& output_masks);

    /// Parallel erase an array of keys in Tensor.
    /// The output_masks will be overwritten, to be used
    /// with tensor buffers.
    /// output_masks stores if the erase is a success or failure (key not found
    /// all already erased in another thread).
    void Erase(const Tensor& input_keys, Tensor& output_masks);

    /// Parallel collect all indices in the buffer corresponding to the active
    /// entries in the hash map. Stored in output_buf_indices.
    void GetActiveIndices(Tensor& output_buf_indices) const;

    /// Clear stored map without reallocating the buffers.
    void Clear();

    /// Save active keys to a npz file at 'key'.
    /// The file name should end with 'npz', otherwise 'npz' will be added as an
    /// extension.
    void Save(const std::string& file_name);

    /// Load active keys and values from a npz file that contains 'key'.
    static HashSet Load(const std::string& file_name);

    /// Clone the hash set with buffers.
    HashSet Clone() const;

    /// Convert the hash set to another device.
    HashSet To(const Device& device, bool copy = false) const;

    /// Get the size (number of active entries) of the hash set.
    int64_t Size() const;

    /// Get the capacity of the hash set.
    int64_t GetCapacity() const;

    /// Get the number of buckets of the internal hash set.
    int64_t GetBucketCount() const;

    /// Get the device of the hash set.
    Device GetDevice() const;

    /// Get the key tensor buffer to be used along with buf_indices and masks.
    /// Example:
    /// GetKeyTensor().IndexGet({buf_indices.To(core::Int64).IndexGet{masks}})
    Tensor GetKeyTensor() const;

    /// Return number of elements per bucket.
    std::vector<int64_t> BucketSizes() const;

    /// Return size / bucket_count.
    float LoadFactor() const;

    /// Return the implementation of the device hash backend.
    std::shared_ptr<DeviceHashBackend> GetDeviceHashBackend() const;

private:
    HashSet(const HashMap& internal_hashmap);
    std::shared_ptr<HashMap> internal_;
};

}  // namespace core
}  // namespace open3d
