#include "framework/logger.h"
#include "library/rect.h"
#include "library/voxel.h"

using namespace OpenApoc;

static void check_voxel(Vec3<int> position, VoxelMap &v, bool expected)
{
	if (v.getBit(position) != expected)
	{
		LogError("Unexpected voxel at {%d,%d,%d} - expected %d", position.x, position.y, position.z,
		         expected ? 1 : 0);
		exit(EXIT_FAILURE);
	}
}

static void check_slice(Vec2<int> position, VoxelSlice &s, bool expected)
{
	if (s.getBit(position) != expected)
	{
		LogError("Unexpected voxel at {%d,%d} - expected %d", position.x, position.y,
		         expected ? 1 : 0);
		exit(EXIT_FAILURE);
	}
}

static void test_voxel(Vec3<int> voxel_size)
{
	VoxelMap v{voxel_size};
	if (v.size != voxel_size)
	{
		LogError("Unexpected size {%d,%d,%d}", v.size.x, v.size.y, v.size.z);
		exit(EXIT_FAILURE);
	}
	// Ensure everything is '0' at init, and anything outside the bounds should be '0' too
	for (int z = -16; z < voxel_size.z + 32; z++)
	{
		for (int y = -64; y < voxel_size.y + 64; y++)
		{
			for (int x = -1; x < voxel_size.x + 99; x++)
			{
				check_voxel({x, y, z}, v, false);
			}
		}
	}

	// An empty map should have a center in the 'middle'
	v.calculateCentre();
	if (v.centre != v.size / 2)
	{
		LogError("Unexpected centre {%d,%d,%d} for empty map", v.centre.x, v.centre.y, v.centre.z);
		exit(EXIT_FAILURE);
	}

	// Add a slice with a set voxel:
	auto slice = mksp<VoxelSlice>(Vec2<int>{voxel_size.x, voxel_size.y});

	if (slice->size != Vec2<int>{voxel_size.x, voxel_size.y})
	{
		LogError("Unexpected slice size {%d,%d}", slice->size.x, slice->size.y);
		exit(EXIT_FAILURE);
	}
	// Ensure everything is '0' at init, and anything outside the bounds should be '0' too
	for (int y = -64; y < voxel_size.y + 64; y++)
	{
		for (int x = -1; x < voxel_size.z + 99; x++)
		{
			check_slice({x, y}, *slice, false);
		}
	}

	// Set one bit to true and check that
	Vec2<int> bit_position = {2, 6};
	if (bit_position.x > voxel_size.x)
	{
		bit_position.x = voxel_size.x - 1;
		LogWarning("Clamping bit position x to %d", bit_position.x);
	}
	if (bit_position.y >= voxel_size.y)
	{
		bit_position.y = voxel_size.y - 1;
		LogWarning("Clamping bit position y to %d", bit_position.y);
	}

	slice->setBit(bit_position, true);
	for (int y = -64; y < voxel_size.y + 64; y++)
	{
		for (int x = -1; x < voxel_size.x + 99; x++)
		{
			if (x == bit_position.x && y == bit_position.y)
				check_slice({x, y}, *slice, true);
			else
				check_slice({x, y}, *slice, false);
		}
	}

	// Put that in the map and check that....
	Vec3<int> bit_voxel_position = {bit_position.x, bit_position.y, 14};
	if (bit_voxel_position.z >= voxel_size.z)
	{
		bit_voxel_position.z = voxel_size.z - 1;
		LogWarning("Clamping bit position z to %d", bit_voxel_position.z);
	}
	v.setSlice(bit_voxel_position.z, slice);
	for (int z = -16; z < voxel_size.z + 33; z++)
	{
		for (int y = -64; y < voxel_size.y + 66; y++)
		{
			for (int x = -1; x < voxel_size.x + 1; x++)
			{
				if (x == bit_voxel_position.x && y == bit_voxel_position.y &&
				    z == bit_voxel_position.z)
					check_voxel({x, y, z}, v, true);
				else
					check_voxel({x, y, z}, v, false);
			}
		}
	}

	// The centre of a voxelmap with a single bit should be the same as that bit position
	v.calculateCentre();
	if (v.centre != bit_voxel_position)
	{
		LogError("Unexpected centre {%d,%d,%d} for single-bit map, expected {%d,%d,%d}", v.centre.x,
		         v.centre.y, v.centre.z, bit_voxel_position.x, bit_voxel_position.y,
		         bit_voxel_position.z);
		exit(EXIT_FAILURE);
	}

	// Unset the bit and make sure it's empty again
	slice->setBit(bit_position, false);
	for (int z = -16; z < voxel_size.z + 32; z++)
	{
		for (int y = -64; y < voxel_size.y + 64; y++)
		{
			for (int x = -1; x < voxel_size.x + 99; x++)
			{
				check_voxel({x, y, z}, v, false);
			}
		}
	}
	v.calculateCentre();
	if (v.centre != v.size / 2)
	{
		LogError("Unexpected centre {%d,%d,%d} for reset-to-empty map", v.centre.x, v.centre.y,
		         v.centre.z);
		exit(EXIT_FAILURE);
	}
	// and set the bit again to get back to single-bit-set state
	slice->setBit(bit_position, true);

	// Add a second bit at (first_bit_pos - 2), the centre should then be at (first_bit_pos +
	// second_bit_pos) / 2

	auto bit_2_voxel_position = bit_voxel_position - Vec3<int>{2, 2, 2};

	if (bit_2_voxel_position.x < 0)
	{
		bit_2_voxel_position.x = 0;
		LogWarning("Clamping bit 2 position x to %d", bit_2_voxel_position.x);
	}
	if (bit_2_voxel_position.y < 0)
	{
		bit_2_voxel_position.y = 0;
		LogWarning("Clamping bit 2 position y to %d", bit_2_voxel_position.y);
	}
	if (bit_2_voxel_position.z < 0)
	{
		bit_2_voxel_position.z = 0;
		LogWarning("Clamping bit 2 position z to %d", bit_2_voxel_position.z);
	}
	auto slice2 = mksp<VoxelSlice>(Vec2<int>{voxel_size.x, voxel_size.y});
	if (bit_2_voxel_position.z == bit_voxel_position.z)
	{
		LogWarning("Slice of bit 2 same as bit 1");
		slice2 = slice;
	}

	slice2->setBit({bit_2_voxel_position.x, bit_2_voxel_position.y}, true);

	v.setSlice(bit_2_voxel_position.z, slice2);
	for (int z = -16; z < voxel_size.z + 33; z++)
	{
		for (int y = -64; y < voxel_size.y + 66; y++)
		{
			for (int x = -1; x < voxel_size.x + 1; x++)
			{
				Vec3<int> pos = {x, y, z};
				if (pos == bit_voxel_position || pos == bit_2_voxel_position)
					check_voxel({x, y, z}, v, true);
				else
					check_voxel({x, y, z}, v, false);
			}
		}
	}

	// Now check the centre
	auto expected_centre = (bit_voxel_position + bit_2_voxel_position) / 2;
	v.calculateCentre();
	if (v.centre != expected_centre)
	{
		LogError("Unexpected centre {%d,%d,%d} for 2 bit map, expected {%d,%d,%d}", v.centre.x,
		         v.centre.y, v.centre.z, expected_centre.x, expected_centre.y, expected_centre.z);
		exit(EXIT_FAILURE);
	}

	// Everything looks good
	return;
}

int main(int, char **)
{
	const std::vector<Vec3<int>> voxel_sizes = {
	    {1, 1, 1}, {32, 32, 16}, {33, 32, 16}, {77, 75, 2222},
	};
	for (auto &size : voxel_sizes)
	{
		LogWarning("Testing voxel size {%d,%d,%d}", size.x, size.y, size.z);
		test_voxel(size);
	}
}