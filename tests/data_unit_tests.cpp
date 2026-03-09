#include <catch2/catch_test_macros.hpp>

#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "src/data/atom.h"
#include "src/data/bond.h"
#include "src/data/structure.h"
#include "src/data/structure_operator.h"
#include "src/data/neb_calculation_loader.h"

namespace {
MatrixUnitcell unitcell_identity(double scale = 10.0) {
    return MatrixUnitcell::Identity() * scale;
}
}

TEST_CASE("Atom selection cycles through primary-secondary-unselected", "[atom]") {
    Atom atom(6, 0.0, 0.0, 0.0);

    REQUIRE(atom.select == 0);
    atom.select_atom();
    REQUIRE(atom.select == 1);
    atom.select_atom();
    REQUIRE(atom.select == 2);
    atom.select_atom();
    REQUIRE(atom.select == 0);
}

TEST_CASE("Bond constructor computes direction length and special axis handling", "[bond]") {
    SECTION("Bond aligned with +z has zero angle") {
        Bond bond(Atom(1, 0.0, 0.0, 0.0), Atom(1, 0.0, 0.0, 2.0), 0, 1);
        REQUIRE(bond.length == Catch::Approx(2.0));
        REQUIRE(bond.direction.z() == Catch::Approx(1.0f));
        REQUIRE(bond.angle == Catch::Approx(0.0));
    }

    SECTION("Bond aligned with -z uses gimbal lock branch") {
        Bond bond(Atom(1, 0.0, 0.0, 1.0), Atom(1, 0.0, 0.0, -1.0), 0, 1);
        REQUIRE(bond.direction.z() == Catch::Approx(-1.0f));
        REQUIRE(bond.axis.y() == Catch::Approx(1.0f));
        REQUIRE(bond.angle == Catch::Approx(-M_PI));
    }
}

TEST_CASE("Structure add_eigenmode validates vector count", "[structure]") {
    Structure structure(unitcell_identity());
    structure.add_atom(6, 0.0, 0.0, 0.0);
    structure.add_atom(1, 1.0, 0.0, 0.0);

    REQUIRE_THROWS_AS(structure.add_eigenmode(100.0, {QVector3D(0, 0, 0)}), std::runtime_error);

    structure.add_eigenmode(120.0, {QVector3D(1, 0, 0), QVector3D(0, 1, 0)});
    REQUIRE(structure.get_nr_eigenmodes() == 1);
    REQUIRE(structure.get_eigenmodes().front().eigenvalue == Catch::Approx(120.0));
}

TEST_CASE("Structure selections and freeze state are synchronized", "[structure]") {
    Structure structure(unitcell_identity());
    structure.add_atom(6, 0.0, 0.0, 0.0, true, true, true);
    structure.add_atom(1, 1.0, 0.0, 0.0, true, true, true);

    structure.select_atom(0);
    REQUIRE(structure.get_nr_atoms_primary_buffer() == 1);
    REQUIRE(structure.get_nr_atoms_secondary_buffer() == 0);

    structure.select_atom(0);
    REQUIRE(structure.get_nr_atoms_primary_buffer() == 0);
    REQUIRE(structure.get_nr_atoms_secondary_buffer() == 1);

    structure.select_atom(1);
    REQUIRE(structure.get_nr_atoms_primary_buffer() == 1);

    structure.set_frozen();
    const auto& atoms = structure.get_atoms();
    REQUIRE(atoms[1].selective_dynamics[0] == false);
    REQUIRE(atoms[1].selective_dynamics[1] == false);
    REQUIRE(atoms[1].selective_dynamics[2] == false);

    structure.set_unfrozen();
    REQUIRE(atoms[1].selective_dynamics[0] == true);
    REQUIRE(atoms[1].selective_dynamics[1] == true);
    REQUIRE(atoms[1].selective_dynamics[2] == true);
}

TEST_CASE("StructureOperator z-alignment preserves z-axis for canonical directions", "[structure-operator]") {
    StructureOperator op;

    SECTION("identity for +z") {
        const auto m = op.build_z_align_matrix(QVector3D(0.0, 0.0, 1.0));
        const QVector3D mapped = m.map(QVector3D(0.0, 0.0, 1.0));
        REQUIRE(mapped.x() == Catch::Approx(0.0f).margin(1e-6));
        REQUIRE(mapped.y() == Catch::Approx(0.0f).margin(1e-6));
        REQUIRE(mapped.z() == Catch::Approx(1.0f).margin(1e-6));
    }

    SECTION("180-degree rotation for -z") {
        const auto m = op.build_z_align_matrix(QVector3D(0.0, 0.0, -1.0));
        const QVector3D mapped = m.map(QVector3D(0.0, 0.0, 1.0));
        REQUIRE(mapped.z() == Catch::Approx(-1.0f).margin(1e-5));
    }
}

TEST_CASE("NebCalculationLoader returns informative errors for invalid input", "[neb-loader]") {
    NebCalculationLoader loader;
    QString error;

    SECTION("non-existing root directory") {
        const bool ok = loader.load("/definitely/non-existent-neb-folder", &error);
        REQUIRE_FALSE(ok);
        REQUIRE(error.contains("does not exist"));
    }

    SECTION("insufficient image directories") {
        QTemporaryDir temp;
        REQUIRE(temp.isValid());
        QDir root(temp.path());
        REQUIRE(root.mkdir("00"));
        REQUIRE(root.mkdir("01"));

        const bool ok = loader.load(temp.path(), &error);
        REQUIRE_FALSE(ok);
        REQUIRE(error.contains("No valid VASP NEB folder structure"));
    }

    SECTION("missing OUTCAR in intermediate image") {
        QTemporaryDir temp;
        REQUIRE(temp.isValid());
        QDir root(temp.path());
        REQUIRE(root.mkdir("00"));
        REQUIRE(root.mkdir("01"));
        REQUIRE(root.mkdir("02"));

        const bool ok = loader.load(temp.path(), &error);
        REQUIRE_FALSE(ok);
        REQUIRE(error.contains("does not contain an OUTCAR"));
    }
}
