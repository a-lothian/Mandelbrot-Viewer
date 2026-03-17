#include "benchmark.h"

#ifdef _WIN32
#define HAVE_STRUCT_TIMESPEC
#endif
#include "colour_palette.h"
#include "core_count.h"
#include "inputHandler.h"
#include "mandelbrot.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCRN_WIDTH 1280
#define SCRN_HEIGHT 720

struct BenchScene {
    const char* name;
    double offset_x, offset_y;
    double zoom;
    int iterations;
};

static const struct BenchScene scenes[] = {
    {"Mandelbrot Overview", -0.72, 0.0, 0.0032, 1000},
    {"Satellite Microbrot", 0.356071294, -0.649363720, 0.000000126, 100000},
    {"Whirlpool", -1.351936027, -0.040835814, 0.0000000001, 8500},
    {"Hypercomplexity", 0.381671028, 0.136425822, 0.0000000003, 32000},
    {"Tendrils", -0.567950683, -0.479570641, 0.0000000001, 17000},
};

#define NUM_SCENES ((int)(sizeof(scenes) / sizeof(scenes[0])))

static double bench_scene(const struct BenchScene* scene, struct BenchmarkOpts opts, struct ThreadPool* tp, struct viewport* vp, Uint32* buffer,
                          Uint32* palette) {
    vp->current_offset_x = scene->offset_x;
    vp->current_offset_y = scene->offset_y;
    vp->zoom = scene->zoom;
    vp->iterations = scene->iterations;

    ATOMIC_BOOL kill = false;
    int rows_per_thread = SCRN_HEIGHT / tp->count;

    for (int i = 0; i < tp->count; i++) {
        tp->jobs[i].start_y = i * rows_per_thread;
        tp->jobs[i].end_y = (i == tp->count - 1) ? SCRN_HEIGHT : (i + 1) * rows_per_thread;
        tp->jobs[i].scrn_width = SCRN_WIDTH;
        tp->jobs[i].vp = vp;
        tp->jobs[i].palette = palette;
        tp->jobs[i].palette_size = PALETTE_SIZE;
        tp->jobs[i].render_smooth = opts.smooth;
        tp->jobs[i].buffer = buffer;
        tp->jobs[i].kill_signal = &kill;
        tp->jobs[i].start_render_frac = 1;
        tp->jobs[i].use_simd = !opts.scalar;
        tp->jobs[i].no_optimisations = opts.no_optimisations;
    }

    struct timespec t0, t1;
    timespec_get(&t0, TIME_UTC);

    for (int i = 0; i < tp->count; i++) {
        pthread_create(&tp->threads[i], NULL, calculateMandelbrotRoutine, &tp->jobs[i]);
    }
    for (int i = 0; i < tp->count; i++) {
        pthread_join(tp->threads[i], NULL);
    }

    timespec_get(&t1, TIME_UTC);

    return (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1e6;
}

static double run_all_scenes(struct BenchmarkOpts opts, long thread_count,
                             Uint32* buffer, struct viewport* vp, Uint32* palette) {
    pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
    struct RenderJob* jobs = malloc(thread_count * sizeof(struct RenderJob));

    if (!threads || !jobs) {
        free(threads);
        free(jobs);
        return -1.0;
    }

    for (int i = 0; i < thread_count; i++) {
        jobs[i].iteration_out = malloc(SCRN_WIDTH * sizeof(int));
        if (!jobs[i].iteration_out) {
            for (int j = 0; j < i; j++)
                free(jobs[j].iteration_out);
            free(threads);
            free(jobs);
            return -1.0;
        }
    }

    struct ThreadPool tp = {
        .threads = threads,
        .jobs = jobs,
        .count = thread_count,
        .kill = false,
    };

    double total_ms = 0.0;
    for (int i = 0; i < NUM_SCENES; i++) {
        total_ms += bench_scene(&scenes[i], opts, &tp, vp, buffer, palette);
    }

    for (int i = 0; i < thread_count; i++)
        free(jobs[i].iteration_out);
    free(threads);
    free(jobs);

    return total_ms;
}

void run_benchmark(struct BenchmarkOpts opts) {
    long thread_count = (opts.threads > 0) ? opts.threads : get_num_logical_cores();

    Uint32* buffer = malloc(sizeof(Uint32) * SCRN_WIDTH * SCRN_HEIGHT);
    struct viewport* vp = init_viewport(SCRN_WIDTH, SCRN_HEIGHT);

    if (!buffer || !vp) {
        fprintf(stderr, "benchmark: allocation failed\n");
        free(buffer);
        free(vp);
        return;
    }

    Uint32 palette[PALETTE_SIZE];
    generateColourPalette(list_palettes[0], 8, palette, PALETTE_SIZE);

    printf("\nMandelbrot Benchmark\n");
    printf("Threads: %ld   Mode: %s\n", thread_count, opts.smooth ? "smooth" : "fast");
    printf("----------------------------------------------------\n");
    printf("%-26s %10s  %12s\n", "Scene", "Time (ms)", "Avg. iter/s (Millions)");
    printf("----------------------------------------------------\n");

    // Need per-scene timings for the detailed table, so allocate the pool here.
    pthread_t* threads = calloc(thread_count, sizeof(pthread_t));
    struct RenderJob* jobs = malloc(thread_count * sizeof(struct RenderJob));

    if (!threads || !jobs) {
        fprintf(stderr, "benchmark: allocation failed\n");
        free(threads);
        free(jobs);
        free(buffer);
        free(vp);
        return;
    }

    for (int i = 0; i < thread_count; i++) {
        jobs[i].iteration_out = malloc(SCRN_WIDTH * sizeof(int));
        if (!jobs[i].iteration_out) {
            fprintf(stderr, "benchmark: iteration_out allocation failed\n");
            for (int j = 0; j < i; j++)
                free(jobs[j].iteration_out);
            free(threads);
            free(jobs);
            free(buffer);
            free(vp);
            return;
        }
    }

    struct ThreadPool tp = {
        .threads = threads,
        .jobs = jobs,
        .count = thread_count,
        .kill = false,
    };

    double total_ms = 0.0;
    double total_iters = 0.0;
    for (int i = 0; i < NUM_SCENES; i++) {
        double ms = bench_scene(&scenes[i], opts, &tp, vp, buffer, palette);
        double scene_iters = (double)SCRN_WIDTH * SCRN_HEIGHT * scenes[i].iterations;
        double avg_iter_s = scene_iters / (ms / 1000.0) / 1e6;
        printf("%-26s %10.1f  %12.1f\n", scenes[i].name, ms, avg_iter_s);
        total_ms += ms;
        total_iters += scene_iters;
    }

    double avg_ms = total_ms / (double)NUM_SCENES;
    double avg_iter_s = total_iters / (total_ms / 1000.0) / 1e6;
    printf("----------------------------------------------------\n");
    printf("%-26s %10.1f  %12.1f\n", "Avg", avg_ms, avg_iter_s);
    printf("%-26s %10.1f  %12s\n", "Total", total_ms, "-");
    printf("----------------------------------------------------\n");
    printf("\nNote: Avg. million iterations/second assumes no bailout, and therefore is an optimistic measurement\n\n");

    for (int i = 0; i < thread_count; i++)
        free(jobs[i].iteration_out);
    free(threads);
    free(jobs);
    free(buffer);
    free(vp);
}

void run_sweep(struct BenchmarkOpts opts) {
    long max_threads = (opts.threads > 0) ? opts.threads : get_num_logical_cores();

    Uint32* buffer = malloc(sizeof(Uint32) * SCRN_WIDTH * SCRN_HEIGHT);
    struct viewport* vp = init_viewport(SCRN_WIDTH, SCRN_HEIGHT);

    if (!buffer || !vp) {
        fprintf(stderr, "benchmark: allocation failed\n");
        free(buffer);
        free(vp);
        return;
    }

    Uint32 palette[PALETTE_SIZE];
    generateColourPalette(list_palettes[0], 8, palette, PALETTE_SIZE);

    printf("\nMandelbrot Thread Sweep  (1-%ld threads, %s mode)\n",
           max_threads, opts.smooth ? "smooth" : "fast");
    printf("------------------------------------------\n");
    printf("%-10s %12s  %10s\n", "Threads", "Total (ms)", "Speedup");
    printf("------------------------------------------\n");

    double baseline_ms = -1.0;
    for (long t = 1; t <= max_threads; t++) {
        double total_ms = run_all_scenes(opts, t, buffer, vp, palette);
        if (total_ms < 0.0) {
            fprintf(stderr, "benchmark: allocation failed for %ld threads\n", t);
            break;
        }
        if (baseline_ms < 0.0) baseline_ms = total_ms;
        printf("%-10ld %12.1f  %9.2fx\n", t, total_ms, baseline_ms / total_ms);
    }

    printf("------------------------------------------\n\n");

    free(buffer);
    free(vp);
}
